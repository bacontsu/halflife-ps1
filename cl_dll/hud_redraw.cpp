/***
*
*	Copyright (c) 1999, Valve LLC. All rights reserved.
*	
*	This product contains software technology licensed from Id 
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// hud_redraw.cpp
//
#include "hud.h"
#include "cl_util.h"
#include <GL/gl.h>
#include <vector>

#include "vgui_TeamFortressViewport.h"

#define MAX_LOGO_FRAMES 56

int grgLogoFrame[MAX_LOGO_FRAMES] =
	{
		1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 13, 13, 13, 13, 13, 12, 11, 10, 9, 8, 14, 15,
		16, 17, 18, 19, 20, 20, 20, 20, 20, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29,
		29, 29, 29, 29, 29, 28, 27, 26, 25, 24, 30, 31};

inline float UTIL_Lerp(float lerpfactor, float A, float B) { return A + lerpfactor * (B - A); }

extern bool g_iVisibleMouse;

float HUD_GetFOV();

extern float IN_GetMouseSensitivity();


namespace
{
	static std::vector<unsigned char> g_psxReadback;
	static std::vector<unsigned char> g_psxReduced;
	static int g_psxBufferWidth = 0;
	static int g_psxBufferHeight = 0;

	// Ordered 4x4 Bayer matrix. The pattern is evaluated in the reduced
	// screen space so the dither remains stable relative to the framebuffer.
	static const unsigned char g_psxBayer4x4[4][4] =
	{
		{ 0,  8,  2, 10 },
		{12,  4, 14,  6 },
		{ 3, 11,  1,  9 },
		{15,  7, 13,  5 }
	};

	static inline unsigned char PSXClampByte(int value)
	{
		if (value < 0)
			return 0;
		if (value > 255)
			return 255;
		return (unsigned char)value;
	}

	static inline unsigned char PSXQuantizeChannel(int value, int x, int y, int levels, float dither)
	{
		if (levels <= 1)
			return 0;

		// Dither before quantization. The threshold is centered around zero.
		const int threshold = (int)g_psxBayer4x4[y & 3][x & 3] - 7;
		value += (int)(threshold * (255.0f / (float)levels) * dither * 0.50f);
		value = (int)PSXClampByte(value);

		// Quantize to the requested number of levels, then expand back to 8-bit
		// because the engine's framebuffer upload path still uses RGBA8 pixels.
		const int q = (value * (levels - 1) + 127) / 255;
		return (unsigned char)((q * 255 + (levels - 1) / 2) / (levels - 1));
	}

	static void ApplyPSXPostProcess()
	{
		const float downsampleCvar = CVAR_GET_FLOAT("te_downsample");
		const float paletteCvar = CVAR_GET_FLOAT("te_palette");
		const float ditherCvar = CVAR_GET_FLOAT("te_dither");

		if (downsampleCvar <= 1.0f && paletteCvar <= 1.0f)
			return;

		const int width = ScreenWidth;
		const int height = ScreenHeight;

		if (width <= 0 || height <= 0)
			return;

		// te_downsample is an integer reduction factor:
		// 1 = native, 2 = half resolution, 4 = quarter resolution, etc.
		int factor = (int)(downsampleCvar + 0.5f);
		if (factor < 1)
			factor = 1;
		if (factor > width)
			factor = width;
		if (factor > height)
			factor = height;

		const int lowWidth = (width + factor - 1) / factor;
		const int lowHeight = (height + factor - 1) / factor;

		const size_t fullSize = (size_t)width * (size_t)height * 4;
		const size_t reducedSize = (size_t)lowWidth * (size_t)lowHeight * 4;

		if (g_psxBufferWidth != width || g_psxBufferHeight != height)
		{
			g_psxReadback.resize(fullSize);
			g_psxReduced.resize(reducedSize);
			g_psxBufferWidth = width;
			g_psxBufferHeight = height;
		}
		else
		{
			if (g_psxReadback.size() != fullSize)
				g_psxReadback.resize(fullSize);
			if (g_psxReduced.size() != reducedSize)
				g_psxReduced.resize(reducedSize);
		}

		glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &g_psxReadback[0]);

		int paletteLevels = (int)(paletteCvar + 0.5f);
		if (paletteLevels < 2)
			paletteLevels = 2;
		if (paletteLevels > 256)
			paletteLevels = 256;

		const float ditherStrength = ditherCvar > 0.0f ? ditherCvar : 0.0f;

		// Downsample first. Each reduced pixel is a box-filtered block from the
		// original framebuffer. Palette reduction and dithering happen afterwards.
		for (int sy = 0; sy < lowHeight; ++sy)
		{
			const int y0 = sy * factor;
			const int y1 = V_min(y0 + factor, height);

			for (int sx = 0; sx < lowWidth; ++sx)
			{
				const int x0 = sx * factor;
				const int x1 = V_min(x0 + factor, width);

				const int sampleX = V_min(x0 + factor / 2, width - 1);
				const int sampleY = V_min(y0 + factor / 2, height - 1);

				const unsigned char* src =
					&g_psxReadback[((size_t)sampleY * width + sampleX) * 4];

				int r = src[0];
				int g = src[1];
				int b = src[2];
				int a = src[3];

				unsigned char* dst = &g_psxReduced[((size_t)sy * lowWidth + sx) * 4];

				if (paletteCvar > 1.0f)
				{
					dst[0] = PSXQuantizeChannel(r, sx, sy, paletteLevels, ditherStrength);
					dst[1] = PSXQuantizeChannel(g, sx, sy, paletteLevels, ditherStrength);
					dst[2] = PSXQuantizeChannel(b, sx, sy, paletteLevels, ditherStrength);
				}
				else
				{
					dst[0] = (unsigned char)r;
					dst[1] = (unsigned char)g;
					dst[2] = (unsigned char)b;
				}

				dst[3] = (unsigned char)a;
			}
		}

		// Scale the reduced framebuffer back to the native display resolution using
		// nearest-neighbour replication. This is the visible low-resolution effect.
		for (int y = 0; y < height; ++y)
		{
			const int sy = y / factor;
			for (int x = 0; x < width; ++x)
			{
				const int sx = x / factor;
				const unsigned char* src = &g_psxReduced[((size_t)sy * lowWidth + sx) * 4];
				unsigned char* dst = &g_psxReadback[((size_t)y * width + x) * 4];

				dst[0] = src[0];
				dst[1] = src[1];
				dst[2] = src[2];
				dst[3] = src[3];
			}
		}

		// Repaint the complete framebuffer with the processed pixels. No shader,
		// texture, or modern OpenGL functionality is required.
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_BLEND);
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_SCISSOR_TEST);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_CULL_FACE);
		glPixelZoom(1.0f, 1.0f);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glOrtho(0, width, 0, height, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();

		glRasterPos2i(0, 0);
		glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, &g_psxReadback[0]);

		glPopMatrix();
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		glMatrixMode(GL_MODELVIEW);
		glPopAttrib();
	}
}

// Think
void CHud::Think()
{
	m_scrinfo.iSize = sizeof(m_scrinfo);
	GetScreenInfo(&m_scrinfo);

	int newfov;
	HUDLIST* pList = m_pHudList;

	while (pList != nullptr)
	{
		if ((pList->p->m_iFlags & HUD_ACTIVE) != 0)
			pList->p->Think();
		pList = pList->pNext;
	}

	newfov = HUD_GetFOV();
	if (newfov == 0)
	{
		m_iFOV = default_fov->value;
	}
	else
	{
		m_iFOV = newfov;
	}

	// the clients fov is actually set in the client data update section of the hud

	// Set a new sensitivity
	if (m_iFOV == default_fov->value)
	{
		// reset to saved sensitivity
		m_flMouseSensitivity = 0;
	}
	else
	{
		// set a new sensitivity that is proportional to the change from the FOV default
		m_flMouseSensitivity = IN_GetMouseSensitivity() * ((float)newfov / (float)V_max(default_fov->value, 90.0f)) * CVAR_GET_FLOAT("zoom_sensitivity_ratio");
	}

	// think about default fov
	if (m_iFOV == 0)
	{ // only let players adjust up in fov,  and only if they are not overriden by something else
		m_iFOV = V_max(default_fov->value, 90);
	}

	if (0 != gEngfuncs.IsSpectateOnly())
	{
		m_iFOV = gHUD.m_Spectator.GetFOV(); // default_fov->value;
	}
}

// Redraw
// step through the local data,  placing the appropriate graphics & text as appropriate
// returns 1 if they've changed, 0 otherwise
bool CHud::Redraw(float flTime, bool intermission)
{
	m_fOldTime = m_flTime; // save time of previous redraw
	m_flTime = flTime;
	m_flTimeDelta = (double)m_flTime - m_fOldTime;
	static float m_flShotTime = 0;

	// Clock was reset, reset delta
	if (m_flTimeDelta < 0)
		m_flTimeDelta = 0;

	// Bring up the scoreboard during intermission
	if (gViewPort != nullptr)
	{
		if (m_iIntermission && !intermission)
		{
			// Have to do this here so the scoreboard goes away
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideScoreBoard();
			gViewPort->UpdateSpectatorPanel();
		}
		else if (!m_iIntermission && intermission)
		{
			m_iIntermission = intermission;
			gViewPort->HideCommandMenu();
			gViewPort->HideVGUIMenu();
			gViewPort->ShowScoreBoard();
			gViewPort->UpdateSpectatorPanel();

			// Take a screenshot if the client's got the cvar set
			if (CVAR_GET_FLOAT("hud_takesshots") != 0)
				m_flShotTime = flTime + 1.0; // Take a screenshot in a second
		}
	}

	if (0 != m_flShotTime && m_flShotTime < flTime)
	{
		gEngfuncs.pfnClientCmd("snapshot\n");
		m_flShotTime = 0;
	}

	m_iIntermission = intermission;

	// if no redrawing is necessary
	// return 0;

	// trigger_viewset stuff
	if (((viewFlags & 1) != 0) && ((viewFlags & 4) != 0)) //AJH Draw the camera hud
	{

		int r, g, b, x, y, a;
		//wrect_t rc;
		SpriteHandle_t m_hCam1;
		int HUD_camera_active;
		int HUD_camera_rect;

		a = 225;

		UnpackRGB(r, g, b, gHUD.m_iHUDColor);
		ScaleColors(r, g, b, a);

		//Draw the flashing camera active logo
		HUD_camera_active = gHUD.GetSpriteIndex("camera_active");
		m_hCam1 = gHUD.GetSprite(HUD_camera_active);
		SPR_Set(m_hCam1, r, g, b);
		x = SPR_Width(m_hCam1, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hCam1, 0) / 2;

		// Draw the camera sprite at 1 fps
		int i = (int)(flTime) % 2;
		i = grgLogoFrame[i] - 1;

		SPR_DrawAdditive(i, x, y, nullptr);

		//Draw the camera reticle (top left)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_tl");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);
		x = ScreenWidth / 4;
		y = ScreenHeight / 4;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));

		//Draw the camera reticle (top right)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_tr");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);

		int w, h;
		w = SPR_Width(m_hCam1, 0) / 2;
		h = SPR_Height(m_hCam1, 0) / 2;

		x = ScreenWidth - ScreenWidth / 4 - w;
		y = ScreenHeight / 4;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));

		//Draw the camera reticle (bottom left)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_bl");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);
		x = ScreenWidth / 4;
		y = ScreenHeight - ScreenHeight / 4 - h;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));

		//Draw the camera reticle (bottom right)
		HUD_camera_rect = gHUD.GetSpriteIndex("camera_rect_br");
		m_hCam1 = gHUD.GetSprite(HUD_camera_rect);
		SPR_Set(m_hCam1, r, g, b);
		x = ScreenWidth - ScreenWidth / 4 - w;
		y = ScreenHeight - ScreenHeight / 4 - h;

		SPR_DrawAdditive(0, x, y, &gHUD.GetSpriteRect(HUD_camera_rect));
	}

	if (((viewFlags & 1) != 0) && ((viewFlags & 2) == 0)) // custom view active, and flag "draw hud" isnt set
		return true;

	// draw all registered HUD elements
	if (0 != m_pCvarDraw->value)
	{
		HUDLIST* pList = m_pHudList;

		while (pList != nullptr)
		{
			if (!intermission)
			{
				if ((pList->p->m_iFlags & HUD_ACTIVE) != 0 && (m_iHideHUDDisplay & HIDEHUD_ALL) == 0)
					pList->p->Draw(flTime);
			}
			else
			{ // it's an intermission,  so only draw hud elements that are set to draw during intermissions
				if ((pList->p->m_iFlags & HUD_INTERMISSION) != 0)
					pList->p->Draw(flTime);
			}

			pList = pList->pNext;
		}
	}

	// are we in demo mode? do we need to draw the logo in the top corner?
	if (0 != m_iLogo)
	{
		int x, y, i;

		if (m_hsprLogo == 0)
			m_hsprLogo = LoadSprite("sprites/%d_logo.spr");

		SPR_Set(m_hsprLogo, 250, 250, 250);

		x = SPR_Width(m_hsprLogo, 0);
		x = ScreenWidth - x;
		y = SPR_Height(m_hsprLogo, 0) / 2;

		// Draw the logo at 20 fps
		int iFrame = (int)(flTime * 20) % MAX_LOGO_FRAMES;
		i = grgLogoFrame[iFrame] - 1;

		SPR_DrawAdditive(i, x, y, nullptr);
	}

	/*
	if ( g_iVisibleMouse )
	{
		void IN_GetMousePos( int *mx, int *my );
		int mx, my;

		IN_GetMousePos( &mx, &my );
		
		if (m_hsprCursor == 0)
		{
			char sz[256];
			sprintf( sz, "sprites/cursor.spr" );
			m_hsprCursor = SPR_Load( sz );
		}

		SPR_Set(m_hsprCursor, 250, 250, 250 );
		
		// Draw the logo at 20 fps
		SPR_DrawAdditive( 0, mx, my, NULL );
	}
	*/

	ApplyPSXPostProcess();

	return true;
}

void ScaleColors(int& r, int& g, int& b, int a)
{
	float x = (float)a / 255;
	r = (int)(r * x);
	g = (int)(g * x);
	b = (int)(b * x);
}

int CHud::DrawHudString(int xpos, int ypos, int iMaxX, const char* szIt, int r, int g, int b)
{
	return xpos + gEngfuncs.pfnDrawString(xpos, ypos, szIt, r, g, b);
}

int CHud::DrawHudNumberString(int xpos, int ypos, int iMinX, int iNumber, int r, int g, int b)
{
	char szString[32];
	sprintf(szString, "%d", iNumber);
	return DrawHudStringReverse(xpos, ypos, iMinX, szString, r, g, b);
}

// draws a string from right to left (right-aligned)
int CHud::DrawHudStringReverse(int xpos, int ypos, int iMinX, const char* szString, int r, int g, int b)
{
	return xpos - gEngfuncs.pfnDrawStringReverse(xpos, ypos, szString, r, g, b);
}

int CHud::DrawHudNumber(int x, int y, int iFlags, int iNumber, int r, int g, int b)
{
	int iWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;
	int k;

	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			k = iNumber / 100;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if ((iFlags & DHN_3DIGITS) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
			x += iWidth;
		}
		else if ((iFlags & (DHN_3DIGITS | DHN_2DIGITS)) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		SPR_Set(GetSprite(m_HUD_number_0 + k), r, g, b);
		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0 + k));
		x += iWidth;
	}
	else if ((iFlags & DHN_DRAWZERO) != 0)
	{
		SPR_Set(GetSprite(m_HUD_number_0), r, g, b);

		// SPR_Draw 100's
		if ((iFlags & DHN_3DIGITS) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		if ((iFlags & (DHN_3DIGITS | DHN_2DIGITS)) != 0)
		{
			//SPR_DrawAdditive( 0, x, y, &rc );
			x += iWidth;
		}

		// SPR_Draw ones

		SPR_DrawAdditive(0, x, y, &GetSpriteRect(m_HUD_number_0));
		x += iWidth;
	}

	return x;
}


int CHud::GetNumWidth(int iNumber, int iFlags)
{
	if ((iFlags & DHN_3DIGITS) != 0)
		return 3;

	if ((iFlags & DHN_2DIGITS) != 0)
		return 2;

	if (iNumber <= 0)
	{
		if ((iFlags & DHN_DRAWZERO) != 0)
			return 1;
		else
			return 0;
	}

	if (iNumber < 10)
		return 1;

	if (iNumber < 100)
		return 2;

	return 3;
}

int CHud::GetHudNumberWidth(int number, int width, int flags)
{
	const int digitWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;

	int totalDigits = 0;

	if (number > 0)
	{
		totalDigits = static_cast<int>(log10(number)) + 1;
	}
	else if ((flags & DHN_DRAWZERO) != 0)
	{
		totalDigits = 1;
	}

	totalDigits = V_max(totalDigits, width);

	return totalDigits * digitWidth;
}

int CHud::DrawHudNumberReverse(int x, int y, int number, int flags, int r, int g, int b)
{
	if (number > 0 || (flags & DHN_DRAWZERO) != 0)
	{
		const int digitWidth = GetSpriteRect(m_HUD_number_0).right - GetSpriteRect(m_HUD_number_0).left;

		int remainder = number;

		do
		{
			const int digit = remainder % 10;
			const int digitSpriteIndex = m_HUD_number_0 + digit;

			//This has to happen *before* drawing because we're drawing in reverse
			x -= digitWidth;

			SPR_Set(GetSprite(digitSpriteIndex), r, g, b);
			SPR_DrawAdditive(0, x, y, &GetSpriteRect(digitSpriteIndex));

			remainder /= 10;
		} while (remainder > 0);
	}

	return x;
}

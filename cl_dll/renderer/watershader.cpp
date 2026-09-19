/*
Trinity Rendering Engine - Classic Water Ripples
*/

#if defined(_WIN32)
#include "windows.h"
#include <GL/glu.h>
#endif
#include "hud.h"
#include "cl_util.h"

#include "const.h"
#include "studio.h"
#include "entity_state.h"
#include "triangleapi.h"
#include "event_api.h"
#include "pm_defs.h"

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <math.h>
#include <algorithm>

#include "propmanager.h"
#include "particle_engine.h"
#include "bsprenderer.h"
#include "watershader.h"

#include "r_efx.h"
#include "r_studioint.h"
#include "studio_util.h"
#include "event_api.h"
#include "event_args.h"
#include "FranUtils/FranUtils_FileSystem.hpp"

extern engine_studio_api_s IEngineStudio;
extern float turbsin[]; // Reusing the turbulation sine wave from bsprenderer.cpp

//CWaterShader gWaterShader;

/*
====================
Init
====================
*/
void CWaterShader::Init()
{
	m_pCvarWaterRipple = gEngfuncs.pfnRegisterVariable("te_water_ripple", "1", FCVAR_ARCHIVE);
	m_pCvarWaterRippleUpdate = gEngfuncs.pfnRegisterVariable("te_water_ripple_updatetime", "0.05", FCVAR_ARCHIVE);
	m_pCvarWaterRippleSpawn = gEngfuncs.pfnRegisterVariable("te_water_ripple_spawntime", "0.1", FCVAR_ARCHIVE);
}

/*
====================
ClearEntities
====================
*/
void CWaterShader::ClearEntities()
{
	if (m_iNumWaterEntities == 0 && m_PixBuffers.empty())
		return;

	// Free pixel backup caches
	for (auto& pair : m_PixBuffers)
		delete[] pair.second;
	m_PixBuffers.clear();

	// Delete dynamic ripple textures
	for (auto& pair : m_RippleTextures)
		glDeleteTextures(1, &pair.second);
	
	m_RippleTextures.clear();
	m_RippleUpdates.clear();

	for (int i = 0; i < m_iNumWaterEntities; i++)
	{
		free(m_pWaterEntities[i].surfaces);
	}

	memset(m_pWaterEntities, 0, sizeof(m_pWaterEntities));
	m_iNumWaterEntities = 0;
}

/*
====================
Shutdown
====================
*/
void CWaterShader::Shutdown()
{
	ClearEntities();
}

/*
====================
VidInit
====================
*/
void CWaterShader::VidInit()
{
	ClearEntities();

	curbuf = buf[0];
	oldbuf = buf[1];
	m_time = m_oldtime = gEngfuncs.GetClientTime() - 0.1;
	memset(buf, 0, sizeof(buf));
}

/*
====================
Restore
====================
*/
void CWaterShader::Restore()
{
	if (m_iNumWaterEntities == 0)
		return;

	// End of frame, so reset fog
	gHUD.m_pFogSettings = m_pMainFogSettings;
}

/*
====================
LoadScript
====================
*/
void CWaterShader::LoadScript()
{
	const std::string& mapScriptName = std::string("scripts/water_") + FilenameFromPath(gEngfuncs.pfnGetLevelName()) + ".txt";
	FranUtils::FileSystem::StringMap outputData;

	bool result = FranUtils::FileSystem::ParseBasicFile(mapScriptName, outputData);

	if (!result)
		result = FranUtils::FileSystem::ParseBasicFile("scripts/water_default.txt", outputData);

	if (!result)
	{
		memset(&m_pWaterFogSettings, 0, sizeof(fog_settings_t));
		return;
	}

	if (!outputData.contains("colr"))
		m_pWaterFogSettings.color[0] = std::stof(outputData.at("colr")) / 255.0f;
	if (!outputData.contains("colg"))
		m_pWaterFogSettings.color[1] = std::stof(outputData.at("colg")) / 255.0f;
	if (!outputData.contains("colb"))
		m_pWaterFogSettings.color[2] = std::stof(outputData.at("colb")) / 255.0f;
	if (!outputData.contains("fogend"))
		m_pWaterFogSettings.end = std::stof(outputData.at("fogend"));
	if (!outputData.contains("fogstart"))
		m_pWaterFogSettings.start = std::stof(outputData.at("fogstart"));

	m_pWaterFogSettings.affectsky = true;
	m_pWaterFogSettings.active = (m_pWaterFogSettings.end >= 1 || m_pWaterFogSettings.start >= 1);
}

/*
====================
AddEntity
====================
*/
void CWaterShader::AddEntity(cl_entity_t* entity)
{
	if (m_iNumWaterEntities == MAX_WATER_ENTITIES)
		return;

	for (int i = 0; i < m_iNumWaterEntities; i++)
	{
		if (m_pWaterEntities[i].entity == entity)
			return; // Already in cache
	}

	cl_water_t* pWater = &m_pWaterEntities[m_iNumWaterEntities];
	pWater->index = m_iNumWaterEntities;
	m_iNumWaterEntities++;

	int isurfacecount = 0;
	msurface_t* psurfaces = entity->model->surfaces + entity->model->firstmodelsurface;
	for (int i = 0; i < entity->model->nummodelsurfaces; i++)
	{
		int j = 0;
		for (; j < psurfaces[i].polys->numverts; j++)
		{
			if (psurfaces[i].polys->verts[0][2] != (entity->curstate.maxs.z - 1))
				break;
		}

		if (j != psurfaces[i].polys->numverts)
			continue;

		if ((psurfaces[i].flags & SURF_PLANEBACK) != 0)
			continue;

		if (psurfaces[i].plane->normal[2] != 1)
			continue;

		isurfacecount++;
	}

	if (isurfacecount == 0)
	{
		memset(&m_pWaterEntities[m_iNumWaterEntities - 1], 0, sizeof(cl_water_t));
		m_iNumWaterEntities--;
		return;
	}

	pWater->surfaces = (msurface_t**)malloc(sizeof(msurface_t*) * isurfacecount);

	for (int i = 0; i < entity->model->nummodelsurfaces; i++)
	{
		int j = 0;
		for (; j < psurfaces[i].polys->numverts; j++)
		{
			if (psurfaces[i].polys->verts[0][2] != (entity->curstate.maxs.z - 1))
				break;
		}

		if (j != psurfaces[i].polys->numverts) continue;
		if ((psurfaces[i].flags & SURF_PLANEBACK) != 0) continue;
		if (psurfaces[i].plane->normal[2] != 1) continue;

		pWater->surfaces[pWater->numsurfaces] = &psurfaces[i];
		pWater->numsurfaces++;
	}

	pWater->mins = Vector(9999, 9999, 9999);
	pWater->maxs = Vector(-9999, -9999, -9999);

	for (int i = 0; i < pWater->numsurfaces; i++)
	{
		for (glpoly_t* bp = pWater->surfaces[i]->polys; bp != nullptr; bp = bp->next)
		{
			for (int j = 0; j < bp->numverts; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					if (pWater->mins[k] > bp->verts[j][k]) pWater->mins[k] = bp->verts[j][k];
					if (pWater->maxs[k] < bp->verts[j][k]) pWater->maxs[k] = bp->verts[j][k];
				}
			}
		}
	}

	pWater->entity = entity;
	pWater->entity->efrag = (efrag_s*)pWater;

	pWater->wplane.dist = psurfaces->plane->dist;
	pWater->wplane.normal[2] = 1;

	pWater->origin = (pWater->mins + pWater->maxs) * 0.5f;
}

/*
====================
SpawnNewRipple
====================
*/
void CWaterShader::SpawnNewRipple(int x, int y, short val)
{
#define PIXEL(x, y) (((x) & RIPPLES_CACHEWIDTH_MASK) + (((y) & RIPPLES_CACHEWIDTH_MASK) << 7))
	oldbuf[PIXEL(x, y)] += val;
	val >>= 2;
	oldbuf[PIXEL(x + 1, y)] += val;
	oldbuf[PIXEL(x - 1, y)] += val;
	oldbuf[PIXEL(x, y + 1)] += val;
	oldbuf[PIXEL(x, y - 1)] += val;
#undef PIXEL
}

/*
====================
RunRipplesAnimation
====================
*/
void CWaterShader::RunRipplesAnimation(const short* poldbuf, short* pbuf)
{
	const int w = RIPPLES_CACHEWIDTH;
	const int m = RIPPLES_TEXSIZE_MASK;

	for (size_t i = w; i < m + w; i++, pbuf++)
	{
		*pbuf = (((int)poldbuf[(i - (w * 2)) & m] + (int)poldbuf[(i - (w + 1)) & m] + (int)poldbuf[(i - (w - 1)) & m] + (int)poldbuf[(i)&m]) >> 1) - (int)*pbuf;
		*pbuf -= (*pbuf >> 6);
	}
}

/*
====================
AnimateRipples
====================
*/
void CWaterShader::AnimateRipples()
{
	double frametime = gEngfuncs.GetClientTime() - m_time;
	m_update = m_pCvarWaterRipple->value > 0 && frametime >= m_pCvarWaterRippleUpdate->value;

	if (!m_update) return;
	m_time = gEngfuncs.GetClientTime();

	short* tempbufp = curbuf;
	curbuf = oldbuf;
	oldbuf = tempbufp;

	if (m_time - m_oldtime > m_pCvarWaterRippleSpawn->value)
	{
		m_oldtime = m_time;
		SpawnNewRipple(rand() & 0x7fff, rand() & 0x7fff, rand() & 0x3ff);
	}

	RunRipplesAnimation(oldbuf, curbuf);
}

/*
====================
GetRippleTextureSize
====================
*/
void CWaterShader::GetRippleTextureSize(const texture_t* image, int* width, int* height)
{
	if (image->width > image->height)
	{
		*width = RIPPLES_CACHEWIDTH;
		*height = (float)image->height / image->width * RIPPLES_CACHEWIDTH;
	}
	else if (image->width < image->height)
	{
		*width = (float)image->width / image->height * RIPPLES_CACHEWIDTH;
		*height = RIPPLES_CACHEWIDTH;
	}
	else
	{
		*width = *height = RIPPLES_CACHEWIDTH;
	}
}

/*
====================
GetPixelBuffer
====================
*/
uint32_t* CWaterShader::GetPixelBuffer(texture_t* image)
{
	gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB);

	auto i = m_PixBuffers.find(image->gl_texturenum);
	if (i != m_PixBuffers.end())
		return i->second;

	int bufsize = image->width * image->height * 4;
	uint32_t* buf = new uint32_t[bufsize];
	memset(buf, 0, bufsize);

	// Backup current texture bind to be non-destructive
	int currentBinding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &currentBinding);

	glBindTexture(GL_TEXTURE_2D, image->gl_texturenum);
	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);

	glBindTexture(GL_TEXTURE_2D, currentBinding);

	m_PixBuffers.insert(std::make_pair(image->gl_texturenum, buf));
	return buf;
}

/*
====================
UploadRipples
====================
*/
bool CWaterShader::UploadRipples(texture_t* image)
{
	if (m_pCvarWaterRipple->value < 1)
		return false;

	uint32_t* pixels = GetPixelBuffer(image);
	if (!pixels)
		return false;

	GLuint& fb_texturenum = m_RippleTextures[image->gl_texturenum];
	unsigned long& dt_texturenum = m_RippleUpdates[image->gl_texturenum];

	int width, height;
	GetRippleTextureSize(image, &width, &height);

	if (fb_texturenum == 0)
	{
		glGenTextures(1, &fb_texturenum);
		int bufsize = width * height * 4;
		uint32_t* buf = new uint32_t[bufsize];
		memset(buf, 0, bufsize);

		glBindTexture(GL_TEXTURE_2D, fb_texturenum);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, buf);
		delete[] buf;

		dt_texturenum = (gBSPRenderer.m_iFrameCount - 1);
		m_update = true;
	}

	glBindTexture(GL_TEXTURE_2D, fb_texturenum);

	if (!m_update || dt_texturenum == gBSPRenderer.m_iFrameCount)
		return true;

	dt_texturenum = gBSPRenderer.m_iFrameCount;
	int size = m_pCvarWaterRipple->value == 1.0f ? 64 : RIPPLES_CACHEWIDTH;

	for (int y = 0; y < height; y++)
	{
		int ry = (float)y / height * size;
		for (int x = 0; x < width; x++)
		{
			int rx = (float)x / width * size;
			int val = curbuf[ry * RIPPLES_CACHEWIDTH + rx] / 16;

			int rpy = (y - val) % height;
			int rpx = (x + val) % width;

			int py = (float)rpy / height * image->height;
			int px = (float)rpx / width * image->width;

			if (py < 0) py = image->height + py;
			if (px < 0) px = image->width + px;

			texture[y * width + x] = pixels[py * image->width + px];
		}
	}

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, texture);
	return true;
}

/*
====================
EmitWaterPolys
====================
*/
void CWaterShader::EmitWaterPolys(msurface_t* warp, bool reverse, bool ripples, cl_entity_t* ent)
{
	if (!warp->polys)
		return;

	float *v, nv, waveHeight;
	float s, t, os, ot;
	float fltime = gEngfuncs.GetClientTime();

	if (warp->polys->verts[0][2] >= m_vViewOrigin[2])
		waveHeight = -ent->curstate.scale;
	else
		waveHeight = ent->curstate.scale;

	Vector absmax = ent->origin + ent->curstate.maxs;

	for (glpoly_t* p = warp->polys; p; p = p->next)
	{
		int numverts = p->numverts;
		if (numverts < 0) numverts = -numverts;

		if (reverse) v = p->verts[0] + (numverts - 1) * VERTEXSIZE;
		else v = p->verts[0];

		glBegin(GL_POLYGON);
		for (int i = 0; i < numverts; i++)
		{
			if (ent != gEngfuncs.GetEntityByIndex(0) && v[2] < absmax.z - 1.0f)
			{
				if (reverse) v -= VERTEXSIZE;
				else v += VERTEXSIZE;
				continue;
			}

			if (waveHeight != 0.0f)
			{
				nv = turbsin[(int)(fltime * 160.0f + v[1] + v[0]) & 255] + 8.0f;
				nv = (turbsin[(int)(v[0] * 5.0f + fltime * 171.0f - v[1]) & 255] + 8.0f) * 0.8f + nv;
				nv = nv * waveHeight + v[2];
			}
			else
			{
				nv = v[2];
			}

			os = v[3];
			ot = v[4];

			if (!ripples)
			{
				s = os + turbsin[(int)((ot * 0.125f + fltime) * 40.0f) & 255];
				t = ot + turbsin[(int)((os * 0.125f + fltime) * 40.0f) & 255];
			}
			else
			{
				s = os;
				t = ot;
			}

			s *= (1.0f / 64.0f);
			t *= (1.0f / 64.0f);

			glTexCoord2f(s, t);
			glVertex3f(v[0], v[1], nv);

			if (reverse) v -= VERTEXSIZE;
			else v += VERTEXSIZE;
		}
		glEnd();
	}
}

/*
====================
DrawWater
====================
*/
void CWaterShader::DrawWater()
{
	if (m_iNumWaterEntities == 0)
		return;

	m_vViewOrigin = gBSPRenderer.m_vRenderOrigin; // Fixes broken wave height
	gBSPRenderer.glActiveTextureARB(GL_TEXTURE0_ARB); 

	// Calculate and upload texture displacements
	AnimateRipples();

	glEnable(GL_BLEND);
	glDisable(GL_ALPHA_TEST);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	// Safe bind handling
	gBSPRenderer.SetTexEnvs(ENVSTATE_REPLACE);

	for (int i = 0; i < m_iNumWaterEntities; i++)
	{
		m_pCurWater = &m_pWaterEntities[i];

		if (!m_pWaterEntities[i].draw)
			continue;

		if (gHUD.viewFrustum.CullBox(m_pCurWater->mins, m_pCurWater->maxs))
			continue;

		cl_entity_t* ent = m_pCurWater->entity;

		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();

		glTranslatef(ent->origin[0], ent->origin[1], ent->origin[2]);
		glRotatef(ent->angles[1], 0, 0, 1);
		glRotatef(-ent->angles[0], 0, 1, 0);
		glRotatef(ent->angles[2], 1, 0, 0);

		float blend = ent->curstate.renderamt / 255.0f;
		if (ent->curstate.rendermode == kRenderTransAdd)
		{
			glBlendFunc(GL_ONE, GL_ONE);
			glColor4f(blend, blend, blend, 1.0f);
		}
		else if (ent->curstate.rendermode == kRenderTransColor)
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4ub(ent->curstate.rendercolor.r, ent->curstate.rendercolor.g, ent->curstate.rendercolor.b, ent->curstate.renderamt);
		}
		else
		{
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glColor4f(1.0f, 1.0f, 1.0f, blend);
		}

		Vector absmax = ent->origin + ent->curstate.maxs;
		bool underwater = m_vViewOrigin[2] < absmax.z;

		for (int j = 0; j < m_pCurWater->numsurfaces; j++)
		{
			msurface_t* surf = m_pCurWater->surfaces[j];
			bool hasRipples = UploadRipples(surf->texinfo->texture);

			if (!hasRipples)
				glBindTexture(GL_TEXTURE_2D, surf->texinfo->texture->gl_texturenum);
			
			EmitWaterPolys(surf, underwater, hasRipples, ent);
		}

		glPopMatrix();
	}

	glDisable(GL_BLEND);
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
}
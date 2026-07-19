/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Water Shader
Written by Andrew Lucas
*/

#if !defined(WATERSHADER_H)
#define WATERSHADER_H
#if defined(_WIN32)
#pragma once
#include "windows.h"
#endif

#include "GL/gl.h"
#include "pm_defs.h"
#include "cl_entity.h"
#include "ref_params.h"
#include "dlight.h"
#include "parsemsg.h"
#include "cvardef.h"
#include "textureloader.h"
#include "rendererdefs.h"
#include "glslshader.h"
#include "watershaderdefs.h"

#include <deque>
#include <unordered_map>

// Uniforms of the Water Shaders
struct glsl_water_uniforms_t
{
	GLint radialfog;
	GLint fogenabled;
	GLint vieworigin; // Above water only
	GLint watercolor;
	GLint fresnel; // Above water only
	GLint time;

	GLint waveheight;
	GLint wavefreq;
	GLint wavespeed;
	GLint quality;
};

// Per-entity Water Settings.
// Received from the Server's func_water via the WaterInfo message.
struct water_settings_t
{
	fog_settings_t fog;
	float fresnel;

	float waveheight;
	float wavefreq;
	float wavespeed;

	water_settings_t()
	{
		fog.color.x = WATER_DEFAULT_COLOR_R / 255.0f;
		fog.color.y = WATER_DEFAULT_COLOR_G / 255.0f;
		fog.color.z = WATER_DEFAULT_COLOR_B / 255.0f;
		fog.start = WATER_DEFAULT_FOG_START;
		fog.end = WATER_DEFAULT_FOG_END;
		fog.affectsky = true;
		fog.active = true;

		fresnel = WATER_DEFAULT_FRESNEL;

		waveheight = 0.0f;
		wavefreq = WATER_DEFAULT_WAVE_FREQ;
		wavespeed = WATER_DEFAULT_WAVE_SPEED;
	}
};

/*
====================
CWaterShader

====================
*/
class CWaterShader
{
public:
	void Init();
	void Shutdown();
	void VidInit();
	void Restore();
	void ClearEntities();

	void AddEntity(cl_entity_t* entity);
	void DrawWater();

	void DrawWaterPasses(ref_params_t* pparams);
	void DrawScene(ref_params_t* pparams, bool forcemodels);

	void SetupRefract();
	void FinishRefract();

	void SetupReflect();
	void FinishReflect();

	void SetupClipping(ref_params_t* pparams, bool isrefracting);

	bool ViewInWater();
	bool ShouldReflect(int index);

	Vector GetWaterOrigin(cl_water_t* pwater = nullptr);

	void SetWaterSettings(int entindex, const water_settings_t& settings);
	const water_settings_t& GetWaterSettings(const cl_water_t* pwater);

	int MsgWaterInfo(const char* pszName, int iSize, void* pbuf);

public:
	bool m_bViewInWater;
	Vector m_vViewOrigin;

	// Deque instead of vector.
	// Water entities are referenced by pointer through cl_entity_t::efrag, so growing must not relocate them.
	std::deque<cl_water_t> m_dequeWaterEntities;

	cvar_t* m_pCvarWaterShader;
	cvar_t* m_pCvarWaterDebug;
	cvar_t* m_pCvarWaterQuality;

	cl_texture_t* m_pNormalTexture;
	cl_water_t* m_pCurWater;

	ref_params_t* m_pViewParams;
	ref_params_t m_pWaterParams;

	Vector m_vWaterOrigin;
	Vector m_vWaterPlaneMins;
	Vector m_vWaterPlaneMaxs;
	Vector m_vWaterEntMins;
	Vector m_vWaterEntMaxs;

	int m_iNumPasses;

public:
	CGLSLShader m_waterShaderAbove;
	CGLSLShader m_waterShaderUnder;

	glsl_water_uniforms_t m_waterUniformsAbove;
	glsl_water_uniforms_t m_waterUniformsUnder;

public:
	fog_settings_t m_pMainFogSettings;

	// Water Settings Received from the Server, keyed by Entity Index.
	std::unordered_map<int, water_settings_t> m_mapWaterSettings;
};

extern CWaterShader gWaterShader;
#endif

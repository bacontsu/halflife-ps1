/*
Trinity Rendering Engine - Classic Water Ripples
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
#include <map>
#include <cstdint>

#define RIPPLES_CACHEWIDTH_BITS 7
#define RIPPLES_CACHEWIDTH (1 << RIPPLES_CACHEWIDTH_BITS)
#define RIPPLES_CACHEWIDTH_MASK ((RIPPLES_CACHEWIDTH) - 1)
#define RIPPLES_TEXSIZE (RIPPLES_CACHEWIDTH * RIPPLES_CACHEWIDTH)
#define RIPPLES_TEXSIZE_MASK (RIPPLES_TEXSIZE - 1)

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
	void LoadScript();

	// Classic Ripple Methods
	void AnimateRipples();
	void SpawnNewRipple(int x, int y, short val);
	void RunRipplesAnimation(const short* oldbuf, short* pbuf);
	void GetRippleTextureSize(const texture_t* image, int* width, int* height);
	uint32_t* GetPixelBuffer(texture_t* image);
	bool UploadRipples(texture_t* image);
	void EmitWaterPolys(msurface_t* warp, bool reverse, bool ripples, cl_entity_t* ent);

public:
	bool m_bViewInWater;
	Vector m_vViewOrigin;

	cl_water_t m_pWaterEntities[MAX_WATER_ENTITIES];
	int m_iNumWaterEntities;
	cl_water_t* m_pCurWater;

	cvar_t* m_pCvarWaterRipple;
	cvar_t* m_pCvarWaterRippleUpdate;
	cvar_t* m_pCvarWaterRippleSpawn;

	// Ripple states and buffers
	short buf[2][RIPPLES_TEXSIZE];
	short *curbuf, *oldbuf;
	double m_time;
	double m_oldtime;
	bool m_update;
	uint32_t texture[RIPPLES_TEXSIZE];

	// Safely stores original textures and updated ripple textures without altering engine structs
	std::map<GLuint, uint32_t*> m_PixBuffers;
	std::map<GLuint, GLuint> m_RippleTextures;
	std::map<GLuint, unsigned long> m_RippleUpdates;

public:
	fog_settings_t m_pMainFogSettings;
	fog_settings_t m_pWaterFogSettings;
};

extern CWaterShader gWaterShader;
#endif
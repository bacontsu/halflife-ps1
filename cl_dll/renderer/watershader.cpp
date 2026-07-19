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

#include "propmanager.h"
#include "particle_engine.h"
#include "bsprenderer.h"
#include "watershader.h"
#include "mirrormanager.h"

#include "r_efx.h"
#include "r_studioint.h"
#include "studio_util.h"
#include "event_api.h"
#include "event_args.h"
#include "FranUtils/FranUtils_FileSystem.hpp"

#include "StudioModelRenderer.h"
#include "GameStudioModelRenderer.h"
extern CGameStudioModelRenderer g_StudioRenderer;

extern float sgn(float a);

//===========================================
//	GLSL SHADERS
//===========================================

char water_vertex_shader[] =
	"#version 120\n"
	"uniform int v_radialfog;\n"
	"uniform int v_quality;\n"
	"uniform float v_time;\n"
	"uniform float v_waveheight;\n"
	"uniform float v_wavefreq;\n"
	"uniform float v_wavespeed;\n"
	"varying vec3 v_wavenormal;\n"
	"void main()\n"
	"{\n"
	"	vec4 vert = gl_Vertex;\n"
	"	v_wavenormal = vec3(0.0, 0.0, 1.0);\n"
	"	if (v_waveheight > 0.0)\n"
	"	{\n"
	"		if (v_quality == 0)\n"
	"		{\n"
	"			float wave = sin(v_time * v_wavespeed + vert.x * v_wavefreq);\n"
	"			wave += cos(v_time * v_wavespeed * 0.83 + vert.y * v_wavefreq * 1.27);\n"
	"			vert.z += wave * 0.5 * v_waveheight;\n"
	"		}\n"
	"		else\n"
	"		{\n"
	"			vec2 p = vert.xy;\n"
	"			float t = mod(v_time * v_wavespeed, 6283.18530718);\n"
	"			vec2 d0 = vec2(0.980581, 0.196116);\n"
	"			vec2 d1 = vec2(-0.447214, 0.894427);\n"
	"			vec2 d2 = vec2(0.707107, 0.707107);\n"
	"			vec2 d3 = vec2(-0.894427, -0.447214);\n"
	"			vec2 d4 = vec2(0.242536, -0.970143);\n"
	"			float f0 = v_wavefreq;\n"
	"			float f1 = v_wavefreq * 1.37;\n"
	"			float f2 = v_wavefreq * 1.91;\n"
	"			float f3 = v_wavefreq * 2.63;\n"
	"			float f4 = v_wavefreq * 3.41;\n"
	"			float p0 = dot(p, d0) * f0 + t;\n"
	"			float p1 = dot(p, d1) * f1 + t * 1.21 + 1.7;\n"
	"			float p2 = dot(p, d2) * f2 + t * 0.83 + 3.1;\n"
	"			float p3 = dot(p, d3) * f3 + t * 1.47 + 4.6;\n"
	"			float p4 = dot(p, d4) * f4 + t * 1.13 + 5.8;\n"
	"			float wave = sin(p0) * 0.36 + sin(p1) * 0.25 + sin(p2) * 0.18;\n"
	"			wave += sin(p3) * 0.13 + sin(p4) * 0.08;\n"
	"			vec2 slope = d0 * (cos(p0) * f0 * 0.36);\n"
	"			slope += d1 * (cos(p1) * f1 * 0.25);\n"
	"			slope += d2 * (cos(p2) * f2 * 0.18);\n"
	"			slope += d3 * (cos(p3) * f3 * 0.13);\n"
	"			slope += d4 * (cos(p4) * f4 * 0.08);\n"
	"			vert.z += wave * v_waveheight;\n"
	"			v_wavenormal = normalize(vec3(-slope * v_waveheight, 1.0));\n"
	"		}\n"
	"	}\n"
	"	vec4 eyepos = gl_ModelViewMatrix * vert;\n"
	"	gl_Position = gl_ProjectionMatrix * eyepos;\n"
	"	gl_TexCoord[0] = vec4(gl_MultiTexCoord0.xy * 0.0078125, 0.0, 1.0);\n"
	"	gl_TexCoord[1] = gl_Position;\n"
	"	gl_TexCoord[2] = vec4(vert.xyz, 1.0);\n"
	"	gl_FogFragCoord = (v_radialfog != 0) ? length(eyepos.xyz) : gl_Position.z;\n"
	"}\n";

// Above water shader.
// Distorted reflection and refraction blended with a fresnel term, tinted towards the water colour.
char water_fragment_above[] =
	"#version 120\n"
	"uniform sampler2D normalmap;\n"
	"uniform sampler2D refractmap;\n"
	"uniform sampler2D reflectmap;\n"
	"uniform vec3 v_vieworigin;\n"
	"uniform vec3 v_watercolor;\n"
	"uniform float v_fresnel;\n"
	"uniform float v_time;\n"
	"uniform int v_quality;\n"
	"uniform int v_fogenabled;\n"
	"varying vec3 v_wavenormal;\n"
	"void main()\n"
	"{\n"
	"	vec2 uv = gl_TexCoord[0].xy;\n"
	"	vec3 nsum = texture2D(normalmap, uv + vec2(-0.13, 0.11) * v_time).rgb;\n"
	"	nsum += texture2D(normalmap, uv + vec2(0.2, 0.15) * v_time).rgb;\n"
	"	nsum += texture2D(normalmap, uv + vec2(0.17, 0.15) * v_time).rgb;\n"
	"	nsum += texture2D(normalmap, uv + vec2(-0.14, -0.16) * v_time).rgb;\n"
	"	vec3 detailnormal = normalize(nsum * 0.5 - 1.0);\n"
	"	vec3 normal = detailnormal;\n"
	"	if (v_quality != 0)\n"
	"		normal = normalize(vec3(v_wavenormal.xy + detailnormal.xy * 0.32, max(v_wavenormal.z, 0.2)));\n"
	"	vec2 distort = normal.xy * 0.23;\n"
	"	vec2 ndc = gl_TexCoord[1].xy / gl_TexCoord[1].w;\n"
	"	vec4 refraction = texture2D(refractmap, ndc * 0.5 + distort + 0.5);\n"
	"	vec4 reflection = texture2D(reflectmap, vec2(ndc.x, -ndc.y) * 0.5 + distort + 0.5);\n"
	"	vec3 viewdir = v_vieworigin - gl_TexCoord[2].xyz;\n"
	"	float fresnel;\n"
	"	if (v_quality == 0)\n"
	"		fresnel = clamp(min(viewdir.z / length(viewdir) * v_fresnel * 1.3, 0.97), 0.0, 1.0);\n"
	"	else\n"
	"	{\n"
	"		float facing = clamp(dot(normalize(viewdir), v_wavenormal), 0.0, 1.0);\n"
	"		fresnel = clamp(min(facing * v_fresnel * 1.3, 0.97), 0.0, 1.0);\n"
	"	}\n"
	"	vec4 color = mix(reflection, refraction, fresnel);\n"
	"	float luminance = color.r + color.g + color.b;\n"
	"	color = mix(color, vec4(v_watercolor * luminance / 3.0, 0.17), 0.2);\n"
	"	if (v_fogenabled != 0)\n"
	"	{\n"
	"		float fog = clamp((gl_Fog.end - abs(gl_FogFragCoord)) * gl_Fog.scale, 0.0, 1.0);\n"
	"		color.rgb = mix(gl_Fog.color.rgb, color.rgb, fog);\n"
	"	}\n"
	"	gl_FragColor = color;\n"
	"}\n";

// Underwater shader.
// Distorted refraction faded towards the water colour.
char water_fragment_under[] =
	"#version 120\n"
	"uniform sampler2D normalmap;\n"
	"uniform sampler2D refractmap;\n"
	"uniform vec3 v_watercolor;\n"
	"uniform float v_time;\n"
	"uniform int v_quality;\n"
	"uniform int v_fogenabled;\n"
	"varying vec3 v_wavenormal;\n"
	"void main()\n"
	"{\n"
	"	vec2 uv = gl_TexCoord[0].xy;\n"
	"	vec3 nsum = texture2D(normalmap, uv + vec2(-0.13, 0.11) * v_time).rgb;\n"
	"	nsum += texture2D(normalmap, uv + vec2(0.2, 0.15) * v_time).rgb;\n"
	"	nsum += texture2D(normalmap, uv + vec2(0.17, 0.15) * v_time).rgb;\n"
	"	nsum += texture2D(normalmap, uv + vec2(-0.14, -0.16) * v_time).rgb;\n"
	"	vec3 detailnormal = normalize(nsum * 0.5 - 1.0);\n"
	"	vec3 normal = detailnormal;\n"
	"	if (v_quality != 0)\n"
	"		normal = normalize(vec3(v_wavenormal.xy + detailnormal.xy * 0.32, max(v_wavenormal.z, 0.2)));\n"
	"	vec2 distort = normal.xy * 0.3;\n"
	"	vec2 ndc = gl_TexCoord[1].xy / gl_TexCoord[1].w;\n"
	"	vec4 refraction = texture2D(refractmap, ndc * 0.5 + distort + 0.5);\n"
	"	float luminance = refraction.r + refraction.g + refraction.b;\n"
	"	vec4 color = mix(refraction, vec4(v_watercolor, 1.0), luminance * 0.06666667);\n"
	"	if (v_fogenabled != 0)\n"
	"	{\n"
	"		float fog = clamp((gl_Fog.end - abs(gl_FogFragCoord)) * gl_Fog.scale, 0.0, 1.0);\n"
	"		color.rgb = mix(gl_Fog.color.rgb, color.rgb, fog);\n"
	"	}\n"
	"	gl_FragColor = color;\n"
	"}\n";

static water_vertex_t InterpolateWaterVertex(const float* a, const float* b, const float* c, float u, float v)
{
	water_vertex_t result;
	for (int i = 0; i < 3; i++)
		result.position[i] = a[i] + (b[i] - a[i]) * u + (c[i] - a[i]) * v;

	for (int i = 0; i < 2; i++)
		result.texcoord[i] = a[i + 3] + (b[i + 3] - a[i + 3]) * u + (c[i + 3] - a[i + 3]) * v;

	return result;
}

static void SubdivideWaterTriangle(const float* a, const float* b, const float* c, std::vector<water_vertex_t>& vertices)
{
	const int subdivisions = WATER_HIGH_QUALITY_SUBDIVISIONS;
	const float inverseSubdivisions = 1.0f / (float)subdivisions;

	for (int row = 0; row < subdivisions; row++)
	{
		for (int column = 0; column < subdivisions - row; column++)
		{
			float u = (float)row * inverseSubdivisions;
			float v = (float)column * inverseSubdivisions;

			vertices.push_back(InterpolateWaterVertex(a, b, c, u, v));
			vertices.push_back(InterpolateWaterVertex(a, b, c, u + inverseSubdivisions, v));
			vertices.push_back(InterpolateWaterVertex(a, b, c, u, v + inverseSubdivisions));

			if (row + column + 1 < subdivisions)
			{
				vertices.push_back(InterpolateWaterVertex(a, b, c, u + inverseSubdivisions, v));
				vertices.push_back(InterpolateWaterVertex(a, b, c, u + inverseSubdivisions, v + inverseSubdivisions));
				vertices.push_back(InterpolateWaterVertex(a, b, c, u, v + inverseSubdivisions));
			}
		}
	}
}

static void BuildHighQualityWaterMesh(cl_water_t* water)
{
	std::vector<water_vertex_t> vertices;

	for (msurface_t* surface : water->surfaces)
	{
		for (glpoly_t* polygon = surface->polys; polygon != nullptr; polygon = polygon->next)
		{
			if (polygon->numverts < 3)
				continue;

			vertices.reserve(vertices.size() + (polygon->numverts - 2) * 3 * WATER_HIGH_QUALITY_SUBDIVISIONS * WATER_HIGH_QUALITY_SUBDIVISIONS);

			for (int i = 1; i < polygon->numverts - 1; i++)
			{
				SubdivideWaterTriangle(
					polygon->verts[0],
					polygon->verts[i],
					polygon->verts[i + 1],
					vertices);
			}
		}
	}

	if (vertices.empty())
		return;

	water->highqualityvertexcount = (int)vertices.size();
	gBSPRenderer.glGenBuffersARB(1, &water->highqualitybuffer);
	gBSPRenderer.glBindBufferARB(GL_ARRAY_BUFFER_ARB, water->highqualitybuffer);
	gBSPRenderer.glBufferDataARB(
		GL_ARRAY_BUFFER_ARB,
		sizeof(water_vertex_t) * vertices.size(),
		vertices.data(),
		GL_STATIC_DRAW_ARB);
	gBSPRenderer.glBindBufferARB(GL_ARRAY_BUFFER_ARB, gBSPRenderer.m_uiBufferIndex);
}

//===========================================
//	GLSL SHADERS
//===========================================

/*
====================
Init

====================
*/
void CWaterShader::Init()
{
	// Set up cvar
	m_pCvarWaterShader = gEngfuncs.pfnRegisterVariable("te_water", "1", FCVAR_ARCHIVE);
	m_pCvarWaterDebug = gEngfuncs.pfnRegisterVariable("te_water_debug", "0", 0);
	m_pCvarWaterQuality = gEngfuncs.pfnRegisterVariable("te_water_quality", "1", FCVAR_ARCHIVE);

	if (!gBSPRenderer.m_bShaderSupport)
		return;

	if (!m_waterShaderAbove.CreateProgram("water above", water_vertex_shader, water_fragment_above) ||
		!m_waterShaderUnder.CreateProgram("water under", water_vertex_shader, water_fragment_under))
	{
		gBSPRenderer.m_bShaderSupport = false;
		gBSPRenderer.m_bDontPromptShadersError = false;
		return;
	}

	m_waterUniformsAbove.radialfog = m_waterShaderAbove.GetUniform("v_radialfog");
	m_waterUniformsAbove.fogenabled = m_waterShaderAbove.GetUniform("v_fogenabled");
	m_waterUniformsAbove.vieworigin = m_waterShaderAbove.GetUniform("v_vieworigin");
	m_waterUniformsAbove.watercolor = m_waterShaderAbove.GetUniform("v_watercolor");
	m_waterUniformsAbove.fresnel = m_waterShaderAbove.GetUniform("v_fresnel");
	m_waterUniformsAbove.time = m_waterShaderAbove.GetUniform("v_time");
	m_waterUniformsAbove.waveheight = m_waterShaderAbove.GetUniform("v_waveheight");
	m_waterUniformsAbove.wavefreq = m_waterShaderAbove.GetUniform("v_wavefreq");
	m_waterUniformsAbove.wavespeed = m_waterShaderAbove.GetUniform("v_wavespeed");
	m_waterUniformsAbove.quality = m_waterShaderAbove.GetUniform("v_quality");

	m_waterUniformsUnder.radialfog = m_waterShaderUnder.GetUniform("v_radialfog");
	m_waterUniformsUnder.fogenabled = m_waterShaderUnder.GetUniform("v_fogenabled");
	m_waterUniformsUnder.vieworigin = -1;
	m_waterUniformsUnder.watercolor = m_waterShaderUnder.GetUniform("v_watercolor");
	m_waterUniformsUnder.fresnel = -1;
	m_waterUniformsUnder.time = m_waterShaderUnder.GetUniform("v_time");
	m_waterUniformsUnder.waveheight = m_waterShaderUnder.GetUniform("v_waveheight");
	m_waterUniformsUnder.wavefreq = m_waterShaderUnder.GetUniform("v_wavefreq");
	m_waterUniformsUnder.wavespeed = m_waterShaderUnder.GetUniform("v_wavespeed");
	m_waterUniformsUnder.quality = m_waterShaderUnder.GetUniform("v_quality");

	m_waterShaderAbove.Bind();
	m_waterShaderAbove.SetUniform1i(m_waterShaderAbove.GetUniform("normalmap"), 0);
	m_waterShaderAbove.SetUniform1i(m_waterShaderAbove.GetUniform("refractmap"), 1);
	m_waterShaderAbove.SetUniform1i(m_waterShaderAbove.GetUniform("reflectmap"), 2);

	m_waterShaderUnder.Bind();
	m_waterShaderUnder.SetUniform1i(m_waterShaderUnder.GetUniform("normalmap"), 0);
	m_waterShaderUnder.SetUniform1i(m_waterShaderUnder.GetUniform("refractmap"), 1);

	CGLSLShader::Unbind();
}

/*
====================
ClearEntities

====================
*/
void CWaterShader::ClearEntities()
{
	for (cl_water_t& water : m_dequeWaterEntities)
	{
		glDeleteTextures(1, &water.reflect);
		glDeleteTextures(1, &water.refract);
		if (water.highqualitybuffer != 0)
			gBSPRenderer.glDeleteBuffersARB(1, &water.highqualitybuffer);
	}

	m_dequeWaterEntities.clear();
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
	int iCurrentBinding;
	glGetIntegerv(GL_TEXTURE_BINDING_2D, &iCurrentBinding);

	// Load texture
	m_pNormalTexture = gTextureLoader.LoadTexture("gfx/textures/watershader.tga");
	glBindTexture(GL_TEXTURE_2D, iCurrentBinding);

	if (m_pNormalTexture == nullptr)
	{
		gEngfuncs.pfnClientCmd("escape\n");
		MessageBox(nullptr, "VIDEO ERROR: Could not load 'gfx/textures/watershader.tga'!\n", "ERROR", MB_OK);
		gEngfuncs.pfnClientCmd("quit\n");
	}

	ClearEntities();
	m_mapWaterSettings.clear();
}

/*
====================
Restore

====================
*/
void CWaterShader::Restore()
{
	if (m_pCvarWaterShader->value < 1)
		return;

	if (!gBSPRenderer.m_bShaderSupport)
		return;

	if (m_dequeWaterEntities.empty())
		return;

	if (!m_bViewInWater)
		return;

	// End of frame, so reset
	gHUD.m_pFogSettings = m_pMainFogSettings;
}

/*
====================
MsgWaterInfo

====================
*/
int CWaterShader::MsgWaterInfo(const char* pszName, int iSize, void* pbuf)
{
	BEGIN_READ(pbuf, iSize);

	int entindex = READ_SHORT();

	water_settings_t settings;
	settings.fog.color.x = (float)READ_BYTE() / 255.0f;
	settings.fog.color.y = (float)READ_BYTE() / 255.0f;
	settings.fog.color.z = (float)READ_BYTE() / 255.0f;
	settings.fog.start = READ_SHORT();
	settings.fog.end = READ_SHORT();
	settings.fog.affectsky = true;
	settings.fog.active = (settings.fog.start >= 1 || settings.fog.end >= 1);

	settings.fresnel = (float)READ_SHORT() / 100.0f;
	if (settings.fresnel <= 0)
		settings.fresnel = WATER_DEFAULT_FRESNEL;

	settings.waveheight = (float)READ_SHORT() / 10.0f;
	settings.wavefreq = (float)READ_SHORT() / 1000.0f;
	settings.wavespeed = (float)READ_SHORT() / 100.0f;

	SetWaterSettings(entindex, settings);
	return 1;
}

/*
====================
SetWaterSettings

====================
*/
void CWaterShader::SetWaterSettings(int entindex, const water_settings_t& settings)
{
	m_mapWaterSettings[entindex] = settings;
}

/*
====================
GetWaterSettings

====================
*/
const water_settings_t& CWaterShader::GetWaterSettings(const cl_water_t* pwater)
{
	static const water_settings_t defaultSettings;

	if (pwater == nullptr || pwater->entity == nullptr)
		return defaultSettings;

	const auto it = m_mapWaterSettings.find(pwater->entity->index);
	if (it == m_mapWaterSettings.end())
		return defaultSettings;

	return it->second;
}

/*
====================
ShouldReflect

====================
*/
bool CWaterShader::ShouldReflect(int index)
{
	if (GetWaterOrigin().z > m_vViewOrigin.z)
		return false;

	// Optimization: Try and find a water entity on the same z coord
	for (int i = 0; i < index; i++)
	{
		if (m_dequeWaterEntities[i].draw)
		{
			if (GetWaterOrigin(&m_dequeWaterEntities[i]).z == GetWaterOrigin().z)
				return false;
		}
	}
	return true;
}

/*
====================
AddEntity

====================
*/
void CWaterShader::AddEntity(cl_entity_t* entity)
{
	for (cl_water_t& water : m_dequeWaterEntities)
	{
		if (water.entity == entity)
			return; // Already in cache
	}

	cl_water_t* pWater = &m_dequeWaterEntities.emplace_back();
	pWater->index = (int)m_dequeWaterEntities.size() - 1;

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

		pWater->surfaces.push_back(&psurfaces[i]);
	}

	if (pWater->surfaces.empty())
	{
		m_dequeWaterEntities.pop_back();
		return;
	}

	pWater->mins = Vector(9999, 9999, 9999);
	pWater->maxs = Vector(-9999, -9999, -9999);

	for (msurface_t* pSurface : pWater->surfaces)
	{
		for (glpoly_t* bp = pSurface->polys; bp != nullptr; bp = bp->next)
		{
			for (int j = 0; j < bp->numverts; j++)
			{
				if (pWater->mins[0] > bp->verts[j][0])
					pWater->mins[0] = bp->verts[j][0];

				if (pWater->mins[1] > bp->verts[j][1])
					pWater->mins[1] = bp->verts[j][1];

				if (pWater->mins[2] > bp->verts[j][2])
					pWater->mins[2] = bp->verts[j][2];

				if (pWater->maxs[0] < bp->verts[j][0])
					pWater->maxs[0] = bp->verts[j][0];

				if (pWater->maxs[1] < bp->verts[j][1])
					pWater->maxs[1] = bp->verts[j][1];

				if (pWater->maxs[2] < bp->verts[j][2])
					pWater->maxs[2] = bp->verts[j][2];
			}
		}
	}

	pWater->entity = entity;
	pWater->entity->efrag = (efrag_s*)pWater;
	BuildHighQualityWaterMesh(pWater);

	pWater->wplane.dist = psurfaces->plane->dist;
	pWater->wplane.type = psurfaces->plane->type;
	pWater->wplane.pad[0] = psurfaces->plane->pad[0];
	pWater->wplane.pad[1] = psurfaces->plane->pad[1];
	pWater->wplane.signbits = psurfaces->plane->signbits;
	pWater->wplane.normal[2] = 1;

	pWater->reflect = current_ext_texture_id;
	current_ext_texture_id++;
	pWater->refract = current_ext_texture_id;
	current_ext_texture_id++;

	pWater->origin[0] = (pWater->mins[0] + pWater->maxs[0]) * 0.5f;
	pWater->origin[1] = (pWater->mins[1] + pWater->maxs[1]) * 0.5f;
	pWater->origin[2] = (pWater->mins[2] + pWater->maxs[2]) * 0.5f;

	glBindTexture(GL_TEXTURE_2D, pWater->reflect);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, pWater->refract);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

/*
====================
SetupClipping

====================
*/
void CWaterShader::SetupClipping(ref_params_t* pparams, bool negative)
{
	float dot;
	float eq1[4];
	float eq2[4];
	float projection[16];

	Vector vDist;
	Vector vNorm;

	Vector vForward;
	Vector vRight;
	Vector vUp;

	AngleVectors(pparams->viewangles, vForward, vRight, vUp);
	VectorSubtract(GetWaterOrigin(), pparams->vieworg, vDist);

	VectorInverse(vRight);
	VectorInverse(vUp);

	if (negative)
	{
		DotProductSSE(&eq1[0], vRight, -m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[1], vUp, -m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[2], vForward, -m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[3], vDist, -m_pCurWater->wplane.normal);
	}
	else
	{
		DotProductSSE(&eq1[0], vRight, m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[1], vUp, m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[2], vForward, m_pCurWater->wplane.normal);
		DotProductSSE(&eq1[3], vDist, m_pCurWater->wplane.normal);
	}

	// Change current projection matrix into an oblique projection matrix
	glGetFloatv(GL_PROJECTION_MATRIX, projection);

	eq2[0] = (sgn(eq1[0]) + projection[8]) / projection[0];
	eq2[1] = (sgn(eq1[1]) + projection[9]) / projection[5];
	eq2[2] = -1.0F;
	eq2[3] = (1.0F + projection[10]) / projection[14];

	dot = eq1[0] * eq2[0] + eq1[1] * eq2[1] + eq1[2] * eq2[2] + eq1[3] * eq2[3];

	projection[2] = eq1[0] * (2.0f / dot);
	projection[6] = eq1[1] * (2.0f / dot);
	projection[10] = eq1[2] * (2.0f / dot) + 1.0F;
	projection[14] = eq1[3] * (2.0f / dot);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(projection);

	glMatrixMode(GL_MODELVIEW);
}

/*
====================
ViewInWater

====================
*/
bool CWaterShader::ViewInWater()
{
	Vector mins, maxs;
	for (int i = 0; i < 3; i++)
	{
		mins[i] = m_pCurWater->entity->curstate.origin[i] + m_pCurWater->entity->curstate.mins[i];
		maxs[i] = m_pCurWater->entity->curstate.origin[i] + m_pCurWater->entity->curstate.maxs[i];
	}

	if (m_vViewOrigin[0] > mins[0] && m_vViewOrigin[1] > mins[1] && m_vViewOrigin[2] > mins[2] && m_vViewOrigin[0] < maxs[0] && m_vViewOrigin[1] < maxs[1] && m_vViewOrigin[2] < maxs[2])
		return true;

	return false;
}

/*
====================
DrawWaterPasses

====================
*/
void CWaterShader::DrawWaterPasses(ref_params_t* pparams)
{
	if (m_pCvarWaterShader->value < 1)
		return;

	if (!gBSPRenderer.m_bShaderSupport)
		return;

	if (m_dequeWaterEntities.empty())
		return;

	// Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	m_iNumPasses = NULL;
	m_bViewInWater = false;
	m_pViewParams = pparams;
	m_pMainFogSettings = gHUD.m_pFogSettings;
	gBSPRenderer.m_bMirroring = true;

	VectorCopy(pparams->vieworg, m_vViewOrigin);
	memcpy(&m_pWaterParams, m_pViewParams, sizeof(ref_params_t));

	for (int i = 0; i < (int)m_dequeWaterEntities.size(); i++)
	{
		m_pCurWater = &m_dequeWaterEntities[i];

		if (!m_pCurWater->draw)
			continue;

		gHUD.viewFrustum.SetFrustum(pparams->viewangles, pparams->vieworg, gHUD.m_iFOV, gHUD.m_pFogSettings.end, true);
		if (gHUD.viewFrustum.CullBox(m_pCurWater->mins, m_pCurWater->maxs))
		{
			// YOU MUST DIE
			m_pCurWater->draw = false;
			continue;
		}

		SetupRefract();
		DrawScene(m_pViewParams, true);
		FinishRefract();

		if (ShouldReflect(i))
		{
			SetupReflect();
			DrawScene(&m_pWaterParams, false);
			FinishReflect();
		}
	}

	for (cl_water_t& water : m_dequeWaterEntities)
	{
		m_pCurWater = &water;

		if (ViewInWater())
		{
			gHUD.m_pFogSettings = GetWaterSettings(m_pCurWater).fog;
			m_bViewInWater = true;
			break;
		}
	}

	if (m_pCvarWaterDebug->value != 0.0f)
		gEngfuncs.Con_Printf("A total of %d passes drawn for water shader.\n", m_iNumPasses);

	gBSPRenderer.m_bMirroring = false;
	glViewport(GL_ZERO, GL_ZERO, ScreenWidth, ScreenHeight);
}

/*
====================
DrawScene

====================
*/
void CWaterShader::DrawScene(ref_params_t* pparams, bool isrefracting)
{
	// Set world renderer
	gBSPRenderer.RendererRefDef(pparams);

	// Draw world
	gBSPRenderer.DrawNormalTriangles();

	R_SaveGLStates();

	if ((m_pCvarWaterShader->value > 1) || isrefracting)
	{
		for (int i = 0; i < gBSPRenderer.m_iNumRenderEntities; i++)
		{
			if (gBSPRenderer.m_pRenderEntities[i]->model->type != mod_studio || gBSPRenderer.m_pRenderEntities[i]->index == 0)
				continue;

			if (gBSPRenderer.m_pRenderEntities[i]->player == 0)
			{
				g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
				g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
			}
			else if (gBSPRenderer.m_pRenderEntities[i] != gEngfuncs.GetLocalPlayer())
			{
				entity_state_t* pPlayer = IEngineStudio.GetPlayerState((gBSPRenderer.m_pRenderEntities[i]->index - 1));
				g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
				g_StudioRenderer.StudioDrawPlayer(STUDIO_RENDER, pPlayer);
			}
		}
	}

	if ((m_pCvarWaterShader->value > 1) || isrefracting)
	{
		for (int i = 0; i < gBSPRenderer.m_iNumRenderEntities; i++)
		{
			if (gBSPRenderer.m_pRenderEntities[i]->model->type == mod_studio && gBSPRenderer.m_pRenderEntities[i]->index == 0)
			{
				g_StudioRenderer.m_pCurrentEntity = gBSPRenderer.m_pRenderEntities[i];
				g_StudioRenderer.StudioDrawModel(STUDIO_RENDER);
			}
		}
	}

	// Render any props
	gPropManager.RenderProps();

	// Render any transparent triangles
	gBSPRenderer.DrawTransparentTriangles();

	if ((m_pCvarWaterShader->value > 1) || isrefracting)
		gParticleEngine.DrawParticles();

	if (m_pCvarWaterDebug->value != 0.0f)
	{
		if (isrefracting)
		{
			gEngfuncs.Con_Printf("Water No %d Refract: %d wpolys, %d epolys, %d studio polys drawn\n",
				m_pCurWater->index, gBSPRenderer.m_iWorldPolyCounter, gBSPRenderer.m_iBrushPolyCounter,
				gBSPRenderer.m_iStudioPolyCounter);
		}
		else
		{
			gEngfuncs.Con_Printf("Water No %d Reflect: %d wpolys, %d epolys, %d studio polys drawn\n",
				m_pCurWater->index, gBSPRenderer.m_iWorldPolyCounter, gBSPRenderer.m_iBrushPolyCounter,
				gBSPRenderer.m_iStudioPolyCounter);
		}
	}

	R_RestoreGLStates();
	m_iNumPasses++;
}

/*
====================
SetupRefract

====================
*/
void CWaterShader::SetupRefract()
{
	glCullFace(GL_FRONT);
	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1, 0, 0); // put X going down
	glRotatef(90, 0, 0, 1);	 // put Z going up
	glRotatef(-m_pViewParams->viewangles[2], 1, 0, 0);
	glRotatef(-m_pViewParams->viewangles[0], 0, 1, 0);
	glRotatef(-m_pViewParams->viewangles[1], 0, 0, 1);
	glTranslatef(-m_vViewOrigin[0], -m_vViewOrigin[1], -m_vViewOrigin[2]);

	glViewport(GL_ZERO, GL_ZERO, WATER_RESOLUTION, WATER_RESOLUTION);

	if (GetWaterOrigin().z < m_vViewOrigin[2])
	{
		SetupClipping(m_pViewParams, false);

		gHUD.viewFrustum.SetExtraCullBox(m_pCurWater->entity->curstate.mins, m_pCurWater->entity->curstate.maxs);
		gHUD.m_pFogSettings = GetWaterSettings(m_pCurWater).fog;
	}
	else
	{
		Vector vMins, vMaxs;
		VectorCopy(gBSPRenderer.m_pWorld->maxs, vMaxs);
		VectorCopy(gBSPRenderer.m_pWorld->mins, vMins);
		vMins.z = GetWaterOrigin().z;

		gHUD.viewFrustum.SetExtraCullBox(vMins, vMaxs);
		SetupClipping(m_pViewParams, true);
	}

	RenderFog();
}

/*
====================
FinishRefract

====================
*/
void CWaterShader::FinishRefract()
{
	// Save mirrored image
	glBindTexture(GL_TEXTURE_2D, m_pCurWater->refract);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, WATER_RESOLUTION, WATER_RESOLUTION, 0);

	// Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Restore modelview
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	gHUD.m_pFogSettings = m_pMainFogSettings;

	// Disable culling
	gHUD.viewFrustum.DisableExtraCullBox();
}

/*
====================
SetupReflect

====================
*/
void CWaterShader::SetupReflect()
{
	Vector vForward;
	Vector vMins, vMaxs;
	AngleVectors(m_pViewParams->viewangles, vForward, nullptr, nullptr);

	float flDist = abs(GetWaterOrigin().z - m_vViewOrigin[2]);
	VectorMASSE(m_vViewOrigin, -2 * flDist, m_pCurWater->wplane.normal, m_pWaterParams.vieworg);

	flDist = DotProduct(vForward, -m_pCurWater->wplane.normal);
	VectorMASSE(vForward, -2 * flDist, -m_pCurWater->wplane.normal, vForward);

	m_pWaterParams.viewangles[0] = -asin(vForward[2]) / M_PI * 180;
	m_pWaterParams.viewangles[1] = atan2(vForward[1], vForward[0]) / M_PI * 180;
	m_pWaterParams.viewangles[2] = -m_pViewParams->viewangles[2];

	AngleVectors(m_pWaterParams.viewangles, m_pWaterParams.forward, m_pWaterParams.right, m_pWaterParams.up);
	VectorCopy(m_pWaterParams.viewangles, m_pWaterParams.cl_viewangles);

	glCullFace(GL_FRONT);
	glColor4f(GL_ONE, GL_ONE, GL_ONE, GL_ONE);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glRotatef(-90, 1, 0, 0); // put X going down
	glRotatef(90, 0, 0, 1);	 // put Z going up
	glRotatef(-m_pWaterParams.viewangles[2], 1, 0, 0);
	glRotatef(-m_pWaterParams.viewangles[0], 0, 1, 0);
	glRotatef(-m_pWaterParams.viewangles[1], 0, 0, 1);
	glTranslatef(-m_pWaterParams.vieworg[0], -m_pWaterParams.vieworg[1], -m_pWaterParams.vieworg[2]);

	glViewport(GL_ZERO, GL_ZERO, WATER_RESOLUTION, WATER_RESOLUTION);

	// Cull everything below the water plane
	VectorCopy(gBSPRenderer.m_pWorld->maxs, vMaxs);
	VectorCopy(gBSPRenderer.m_pWorld->mins, vMins);
	vMins.z = GetWaterOrigin().z;

	gHUD.viewFrustum.SetExtraCullBox(vMins, vMaxs);
	SetupClipping(&m_pWaterParams, true);
	RenderFog();
}

/*
====================
FinishReflect

====================
*/
void CWaterShader::FinishReflect()
{
	// Save mirrored image
	glBindTexture(GL_TEXTURE_2D, m_pCurWater->reflect);
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, WATER_RESOLUTION, WATER_RESOLUTION, 0);

	// Completely clear everything
	glClearColor(GL_ZERO, GL_ZERO, GL_ZERO, GL_ONE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_ACCUM_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	// Restore modelview
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	// Turn culling off
	gHUD.viewFrustum.DisableExtraCullBox();
}

/*
====================
DrawWater

====================
*/
void CWaterShader::DrawWater()
{
	if (m_pCvarWaterShader->value < 1)
		return;

	if (!gBSPRenderer.m_bShaderSupport)
		return;

	if (m_dequeWaterEntities.empty())
		return;

	float flTime = gEngfuncs.GetClientTime();
	int iRadialFog = (gBSPRenderer.m_bRadialFogSupport && gBSPRenderer.m_pCvarRadialFog->value > 0) ? 1 : 0;
	int iFogEnabled = gHUD.m_pFogSettings.active ? 1 : 0;
	int iWaterQuality = (m_pCvarWaterQuality->value >= 1.0f) ? 1 : 0;

	gBSPRenderer.EnableVertexArray();
	gBSPRenderer.SetTexPointer(0, TC_TEXTURE);

	for (int i = 0; i < (int)m_dequeWaterEntities.size(); i++)
	{
		m_pCurWater = &m_dequeWaterEntities[i];

		if (!m_pCurWater->draw)
			continue;

		if (gHUD.viewFrustum.CullBox(m_pCurWater->mins, m_pCurWater->maxs))
			continue;

		// The shaders transform with the current matrices.
		// So the entity translation has to stay on the stack while the surfaces are drawn.
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glTranslatef(m_pCurWater->entity->curstate.origin[0], m_pCurWater->entity->curstate.origin[1], m_pCurWater->entity->curstate.origin[2]);

		const water_settings_t& settings = GetWaterSettings(m_pCurWater);

		if (m_vViewOrigin[2] > GetWaterOrigin().z)
		{
			glCullFace(GL_FRONT);
			m_waterShaderAbove.Bind();
			m_waterShaderAbove.SetUniform1i(m_waterUniformsAbove.radialfog, iRadialFog);
			m_waterShaderAbove.SetUniform1i(m_waterUniformsAbove.fogenabled, iFogEnabled);
			m_waterShaderAbove.SetUniform3f(m_waterUniformsAbove.vieworigin, gBSPRenderer.m_vRenderOrigin[0], gBSPRenderer.m_vRenderOrigin[1], gBSPRenderer.m_vRenderOrigin[2]);
			m_waterShaderAbove.SetUniform3f(m_waterUniformsAbove.watercolor, settings.fog.color[0], settings.fog.color[1], settings.fog.color[2]);
			m_waterShaderAbove.SetUniform1f(m_waterUniformsAbove.fresnel, settings.fresnel);
			m_waterShaderAbove.SetUniform1f(m_waterUniformsAbove.time, flTime);
			m_waterShaderAbove.SetUniform1f(m_waterUniformsAbove.waveheight, settings.waveheight);
			m_waterShaderAbove.SetUniform1f(m_waterUniformsAbove.wavefreq, settings.wavefreq);
			m_waterShaderAbove.SetUniform1f(m_waterUniformsAbove.wavespeed, settings.wavespeed);
			m_waterShaderAbove.SetUniform1i(m_waterUniformsAbove.quality, iWaterQuality);
		}
		else
		{
			glCullFace(GL_BACK);
			m_waterShaderUnder.Bind();
			m_waterShaderUnder.SetUniform1i(m_waterUniformsUnder.radialfog, iRadialFog);
			m_waterShaderUnder.SetUniform1i(m_waterUniformsUnder.fogenabled, iFogEnabled);
			m_waterShaderUnder.SetUniform3f(m_waterUniformsUnder.watercolor, settings.fog.color[0], settings.fog.color[1], settings.fog.color[2]);
			m_waterShaderUnder.SetUniform1f(m_waterUniformsUnder.time, flTime);
			m_waterShaderUnder.SetUniform1f(m_waterUniformsUnder.waveheight, settings.waveheight);
			m_waterShaderUnder.SetUniform1f(m_waterUniformsUnder.wavefreq, settings.wavefreq);
			m_waterShaderUnder.SetUniform1f(m_waterUniformsUnder.wavespeed, settings.wavespeed);
			m_waterShaderUnder.SetUniform1i(m_waterUniformsUnder.quality, iWaterQuality);
		}

		gBSPRenderer.Bind2DTexture(GL_TEXTURE0_ARB, m_pNormalTexture->iIndex);
		gBSPRenderer.Bind2DTexture(GL_TEXTURE1_ARB, m_pCurWater->refract);

		// Optimisation: Try and find a water entity on the same z coord
		int j = 0;
		for (; j < i; j++)
		{
			if (m_dequeWaterEntities[j].draw)
			{
				if (GetWaterOrigin(&m_dequeWaterEntities[j]).z == GetWaterOrigin().z)
				{
					gBSPRenderer.Bind2DTexture(GL_TEXTURE2_ARB, m_dequeWaterEntities[j].reflect);
					break;
				}
			}
		}

		if (j == i)
			gBSPRenderer.Bind2DTexture(GL_TEXTURE2_ARB, m_pCurWater->reflect);

		if (iWaterQuality != 0 && m_pCurWater->highqualitybuffer != 0)
		{
			gBSPRenderer.glBindBufferARB(GL_ARRAY_BUFFER_ARB, m_pCurWater->highqualitybuffer);
			glVertexPointer(3, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, position));
			gBSPRenderer.glClientActiveTextureARB(GL_TEXTURE0_ARB);
			glTexCoordPointer(2, GL_FLOAT, sizeof(water_vertex_t), OFFSET(water_vertex_t, texcoord));

			if (gBSPRenderer.m_bSpecialFog)
				glDisableClientState(GL_FOG_COORD_ARRAY);

			glDrawArrays(GL_TRIANGLES, 0, m_pCurWater->highqualityvertexcount);

			gBSPRenderer.glBindBufferARB(GL_ARRAY_BUFFER_ARB, gBSPRenderer.m_uiBufferIndex);
			glVertexPointer(3, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, pos));
			glTexCoordPointer(2, GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, texcoord));

			if (gBSPRenderer.m_bSpecialFog)
			{
				glEnableClientState(GL_FOG_COORD_ARRAY);
				gBSPRenderer.glFogCoordPointer(GL_FLOAT, sizeof(brushvertex_t), OFFSET(brushvertex_t, fogcoord));
			}
		}
		else
		{
			for (msurface_t* pSurface : m_pCurWater->surfaces)
				gBSPRenderer.DrawPolyFromArray(gBSPRenderer.m_pWorld->surfaces, pSurface);
		}

		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
	}

	CGLSLShader::Unbind(); // Fran: Wasted 3 hours debugging this because I forgot to unbind :)
	glCullFace(GL_FRONT);

	gBSPRenderer.DisableVertexArray();
}

/*
====================
GetWaterOrigin

====================
*/
Vector CWaterShader::GetWaterOrigin(cl_water_t* pwater)
{
	if (pwater != nullptr)
		return pwater->origin + pwater->entity->curstate.origin;
	else
		return m_pCurWater->origin + m_pCurWater->entity->curstate.origin;
}

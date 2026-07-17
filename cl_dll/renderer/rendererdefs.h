/*
Trinity Rendering Engine - Copyright Andrew Lucas 2009-2012

The Trinity Engine is free software, distributed in the hope th-
at it will be useful, but WITHOUT ANY WARRANTY; without even the
implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the GNU Lesser General Public License for more det-
ails.

Renderer base definitions and functions
Written by Andrew Lucas, Richard Rohac, BUzer, Laurie, Botman and Id Software
*/

#ifndef RENDERERDEFS_H
#define RENDERERDEFS_H

#if defined(_WIN32)
#include "windows.h"
#endif

#include "GL/gl.h"
#include "gl/glext.h"
#include "dlight.h"
#include "com_model.h"
#include "cl_entity.h"
#include <assert.h>
#include "r_studioint.h"
#include "frustum.h"
#include "studio.h"
#include "pm_defs.h"

#include <vector>
#include <map>
#include <unordered_map>
#include <string>

//==============================
//		SHARED DEFS
//
//==============================
constexpr int MAXRENDERENTS = 4096;

#ifndef M_PI
#define M_PI 3.14159265358979323846 // matches value in gcc v2 math.h
#endif

//==============================
//		TEXTURE LOADER STRUCTS
//
//==============================
struct cl_texture_t
{
	std::string strName;

	GLuint iIndex;

	int iBpp;
	unsigned int iWidth;
	unsigned int iHeight;
};


//==============================
//		PARTICLE ENGINE DEFS
//
//==============================
constexpr int SYSTEM_SHAPE_POINT = 0;
constexpr int SYSTEM_SHAPE_BOX = 1;
constexpr int SYSTEM_SHAPE_PLANE_ABOVE_PLAYER = 2;
constexpr int SYSTEM_SHAPE_BOX_AROUND_PLAYER = 3;

constexpr int SYSTEM_DISPLAY_NORMAL = 0;
constexpr int SYSTEM_DISPLAY_PARALELL = 1;
constexpr int SYSTEM_DISPLAY_PLANAR = 2;
constexpr int SYSTEM_DISPLAY_TRACER = 3;

constexpr int SYSTEM_RENDERMODE_ADDITIVE = 0;
constexpr int SYSTEM_RENDERMODE_ALPHABLEND = 1;
constexpr int SYSTEM_RENDERMODE_INTENSITY = 2;

constexpr int PARTICLE_COLLISION_NONE = 0;
constexpr int PARTICLE_COLLISION_DIE = 1;
constexpr int PARTICLE_COLLISION_BOUNCE = 2;
constexpr int PARTICLE_COLLISION_DECAL = 3;
constexpr int PARTICLE_COLLISION_STUCK = 4;
constexpr int PARTICLE_COLLISION_NEW_SYSTEM = 5;

constexpr int PARTICLE_WIND_NONE = 0;
constexpr int PARTICLE_WIND_LINEAR = 1;
constexpr int PARTICLE_WIND_SINE = 2;

constexpr int PARTICLE_LIGHTCHECK_NONE = 0;
constexpr int PARTICLE_LIGHTCHECK_NORMAL = 1;
constexpr int PARTICLE_LIGHTCHECK_SCOLOR = 2;
constexpr int PARTICLE_LIGHTCHECK_MIXP = 3;

//========================================
//			PARTICLE ENGINE STRUCTS
//
//========================================
struct particle_system_t
{
	int id;
	int shapetype;
	int randomdir;

	Vector origin;
	Vector dir;

	float minvel;
	float maxvel;
	float maxofs;

	float skyheight;

	float spawntime;
	float fadeintime;
	float fadeoutdelay;
	float velocitydamp;
	float stuckdie;
	float tracerdist;

	float maxheight;

	float windx;
	float windy;
	float windvar;
	float windmult;
	float windmultvar;
	int windtype;

	float maxlife;
	float maxlifevar;
	float systemsize;

	Vector primarycolor;
	Vector secondarycolor;
	float transitiondelay;
	float transitiontime;
	float transitionvar;

	float rotationvar;
	float rotationvel;
	float rotationdamp;
	float rotationdampdelay;

	float rotxvar;
	float rotxvel;
	float rotxdamp;
	float rotxdampdelay;

	float rotyvar;
	float rotyvel;
	float rotydamp;
	float rotydampdelay;

	float scale;
	float scalevar;
	float scaledampdelay;
	float scaledampfactor;
	float veldampdelay;
	float gravity;
	float particlefreq;
	float impactdamp;
	float mainalpha;

	int startparticles;
	int maxparticles;
	int maxparticlevar;

	int overbright;
	int lightcheck;
	int collision;
	int colwater;
	int displaytype;
	int rendermode;
	int numspawns;

	int fadedistfar;
	int fadedistnear;

	int numframes;
	int framesizex;
	int framesizey;
	int framerate;

	std::string create;
	std::string deathcreate;
	std::string watercreate;

	particle_system_t* createsystem;
	particle_system_t* watersystem;
	particle_system_t* parentsystem;

	cl_texture_t* texture;
	mleaf_t* leaf;

	particle_system_t* next;
	particle_system_t* prev;

	struct cl_particle_t* particleheader;
};

struct cl_particle_t
{
	Vector velocity;
	Vector origin;
	Vector color;
	Vector scolor;
	Vector lastspawn;
	Vector normal;

	float spawntime;
	float life;
	float scale;
	float alpha;

	float fadeoutdelay;

	float scaledampdelay;
	float secondarydelay;
	float secondarytime;

	float rotationvel;
	float rotation;

	float rotx;
	float rotxvel;

	float roty;
	float rotyvel;

	float windxvel;
	float windyvel;
	float windmult;

	float texcoords[4][2];

	int frame;

	particle_system_t* pSystem;

	cl_particle_t* next;
	cl_particle_t* prev;
};

//==============================
//		BSP RENDERER DEFS
//
//==============================
constexpr int MAX_DECALTEXTURES = 128;
constexpr int MAX_GROUPENTRIES = 64;
constexpr int MAX_LIGHTMAPS = 64;
constexpr int MAX_DYNLIGHTS = 64;
constexpr int MAX_MAP_DETAILOBJECTS = 512;
constexpr int MAX_MAP_LEAFS = 65534;
constexpr int DEPTHMAP_RESOLUTION = 256;
constexpr int MAX_MAP_TEXTURES = 512;
constexpr int LIGHTMAP_RESOLUTION = 1024;
constexpr int LIGHTMAP_NUMCOLUMNS = 8;
constexpr int LIGHTMAP_NUMROWS = 8;
constexpr int MAX_SPOTLIGHT_TEXTURES = 16;

constexpr int MAX_GOLDSRC_DLIGHTS = 32;
constexpr int MAX_GOLDSRC_ELIGHTS = 64;

constexpr int SURF_PLANEBACK = 2;
constexpr int SURF_DRAWSKY = 4;
constexpr int SURF_DRAWSPRITE = 8;
constexpr int SURF_DRAWTURB = 0x10;
constexpr int SURF_DRAWTILED = 0x20;
constexpr int SURF_DRAWBACKGROUND = 0x40;
constexpr int SURF_UNDERWATER = 0x80;
constexpr int SURF_DONTWARP = 0x100;

constexpr int BLOCK_WIDTH = 128;
constexpr int BLOCK_HEIGHT = 128;
constexpr int BLOCKLIGHTS_SIZE = (18 * 18);
constexpr int BACKFACE_EPSILON = 0.01;

constexpr int PLANE_X = 0;
constexpr int PLANE_Y = 1;
constexpr int PLANE_Z = 2;

#define OFFSET(type, variable) ((const void*)&(((type*)NULL)->variable))

// Texture pointer settings
enum
{
	TC_OFF,
	TC_TEXTURE,
	TC_LIGHTMAP,
	TC_VERTEX_POSITION, // for specular and dynamic lighting
	TC_DETAIL_TEXTURE,	// for detail texturing
	TC_NOSTATE			// uninitialized
};

// Envstate settings
enum
{
	ENVSTATE_OFF,
	ENVSTATE_REPLACE,
	ENVSTATE_MUL_CONST,
	ENVSTATE_MUL_PREV_CONST, // ignores texture
	ENVSTATE_MUL,
	ENVSTATE_MUL_X2,
	ENVSTATE_ADD,
	ENVSTATE_DOT,
	ENVSTATE_DOT_CONST,
	ENVSTATE_PREVCOLOR_CURALPHA,
	ENVSTATE_NOSTATE // uninitialized
};

//========================================
//			BSP RENDERER STRUCTS
//
//========================================
struct brushvertex_t
{
	Vector pos;
	Vector normal;

	float fogcoord;
	float texcoord[2];
	float detailtexcoord[2];
	float lightmaptexcoord[2];

	byte pad[12];
};

struct brushface_t
{
	int index;
	int start_vertex;
	int num_vertexes;

	Vector normal;
	Vector s_tangent;
	Vector t_tangent;
};

struct DetailTexture
{
	std::string texname;
	std::string detailtexname;
	int texindex;
	float xscale;
	float yscale;

	DetailTexture(const std::string& _texname = {}, const std::string& _detailtexname = {}, int _texindex = 0, float _xscale = 0.0f, float _yscale = 0.0f) : texname(_texname), detailtexname(_detailtexname), texindex(_texindex), xscale(_xscale), yscale(_yscale) {}
};

struct DecalTexture
{
	std::string name;
	int gl_texid;
	int xsize;
	int ysize;
	std::string group; // Parent group

	DecalTexture(const std::string& _name = {}, int _gl_texid = 0, int _xsize = 0, int _ysize = 0, const std::string& _group = {}) : name(_name), gl_texid(_gl_texid), xsize(_xsize), ysize(_ysize), group(_group) {}
};

using DecalTextureGroup = std::unordered_map<std::string, DecalTexture>;

struct CustomDecalVert
{
	Vector position;
	float texcoord[2];

	CustomDecalVert(const Vector& _position = Vector()) : position(_position) {}
};

struct CustomDecalPoly
{
	std::vector<CustomDecalVert> verts;

	msurface_t* surface;
	cl_entity_t* entity;

	CustomDecalPoly(msurface_t* _surface = nullptr, cl_entity_t* _entity = nullptr) : surface(_surface), entity(_entity) {}
};

struct DecalTextureBinding
{
	// Overengineering at its finest

	std::string decalGroup;
	std::string decalGroupMember;

	DecalTextureBinding(const std::string& _decalGroup = {}, const std::string& _decalGroupMember = {}) : decalGroup(_decalGroup), decalGroupMember(_decalGroupMember) {}

	DecalTextureBinding(const DecalTexture& other) : decalGroup(other.group), decalGroupMember(other.name) {}

	bool operator==(const DecalTextureBinding& other) const
	{
		return decalGroup == other.decalGroup && decalGroupMember == other.decalGroupMember;
	}

	bool operator!=(const DecalTextureBinding& other) const
	{
		return !(*this == other);
	}

	DecalTextureBinding& operator=(const DecalTextureBinding& other)
	{
		decalGroup = other.decalGroup;
		decalGroupMember = other.decalGroupMember;

		return *this;
	}

	bool operator==(const DecalTexture& other) const
	{
		return decalGroup == other.group && decalGroupMember == other.name;
	}

	bool operator!=(const DecalTexture& other) const
	{
		return !(*this == other);
	}

	DecalTextureBinding& operator=(const DecalTexture& other)
	{
		decalGroup = other.group;
		decalGroupMember = other.name;

		return *this;
	}
};

struct CustomDecal
{
	std::vector<CustomDecalPoly> polys;

	Vector normal;
	Vector position;
	float life;

	DecalTextureBinding textureBinding;

	CustomDecal(const Vector& _normal = Vector(), const Vector& _position = Vector(), float _life = 0.0f, const DecalTextureBinding& _textureBinding = DecalTextureBinding()) : normal(_normal), position(_position), life(_life), textureBinding(_textureBinding) {}

	void SetTextureBinding(const DecalTextureBinding& newBinding)
	{
		textureBinding = newBinding;
	}
};

struct DecalMessage
{
	std::string name;
	Vector pos;
	Vector normal;
	bool persistent;

	DecalMessage(const std::string& _name = {}, const Vector& _pos = Vector(), const Vector& _normal = Vector(), bool _persistent = false) : name(_name), pos(_pos), normal(_normal), persistent(_persistent) {}
};

struct clientsurfdata_t
{
	float cached_light[MAXLIGHTMAPS];

	texture_t* regtexture;
	texture_t* mptexture;

	int light_s;
	int light_t;
};

using LightStyle = std::tuple<std::string, int>;

struct detailobject_t
{
	Vector mins;
	Vector maxs;

	int firstsurface;
	int numsurfaces;

	short leafnums[MAX_ENT_LEAFS * 2];
	int numleafs;

	int visframe;
	int rendermode;
};

struct cl_dlight_t
{
	Vector origin;
	Vector color;
	Vector angles;

	float radius;
	float die;
	float decay;
	int key;
	int noshadow;

	GLuint depth;

	// spotlight specific:
	float cone_size;
	FrustumCheck frustum;
	int textureindex;
};

//==================================================
//				WATER SHADER DEFS
//
//==================================================
constexpr int WATER_RESOLUTION = 512;

//==================================================
//				WATER SHADER STRUCTS
//
//==================================================
struct cl_water_t
{
	int index;
	cl_entity_t* entity;

	mplane_t wplane;

	Vector mins;
	Vector maxs;
	Vector origin;
	bool draw;

	GLuint refract;
	GLuint reflect;
	GLuint dbuffer;

	std::vector<msurface_t*> surfaces;

	cl_water_t() : index(0), entity(nullptr), wplane{}, draw(false), refract(0), reflect(0), dbuffer(0) {}
};

//==================================================
//				MIRROR MANAGER DEFS
//
//==================================================
constexpr int MAX_MIRRORS = 32;
constexpr int MIRROR_RESOLUTION = 512;

//==================================================
//				MIRROR MANAGER STRUCTS
//
//==================================================
struct cl_mirror_t
{
	cl_entity_t* entity;

	Vector mins;
	Vector maxs;

	Vector origin;
	msurface_t* surface;

	bool draw;

	GLuint texture;
};
//==============================
//		STUDIO RENDERER DEFS
//
//==============================
constexpr int MAX_MODEL_LIGHTS = 6;
constexpr int MAX_MODEL_DECALS = 16;
constexpr int MAX_CACHE_MODELS = 2048;

constexpr int TEXFLAG_NONE = 1;
constexpr int TEXFLAG_FULLBRIGHT = 1;
constexpr int TEXFLAG_ALTERNATE = 2;
constexpr int TEXFLAG_NOMIPMAP = 4;
constexpr int TEXFLAG_ERASE = 8;

//========================================
//				STUDIO RENDERER STRUCTS
//
//========================================
struct StudioDecalVert
{
	int vertindex;
	float texcoord[2];
};

struct StudioDecalVertInfo
{
	Vector position;
	byte boneindex;
};

//struct StudioDecalPoly
//{
//	StudioDecalVert* verts;
//	int numverts;
//};

// Just a vector of vertices. 
using StudioDecalPoly = std::vector<StudioDecalVert>;

struct StudioDecal
{
	int entindex;

	std::vector<StudioDecalPoly> polys;
	std::vector<StudioDecalVertInfo> verts;
	std::vector<Vector> vertexTransforms;

	DecalTextureBinding textureBinding;
};

struct studiovert_t
{
	int vertindex;
	int normindex;
	int texcoord[2];
	byte boneindex;
};

struct studiotri_t
{
	studiovert_t verts[3];
};

struct mlight_t
{
	Vector origin;
	float radius;
	Vector color;

	bool flashlight;
	Vector forward;
	float spotcos;

	FrustumCheck* frustum;

	Vector mins;
	Vector maxs;
};

struct texentry_t
{
	std::string strModel;
	std::string strTexture;

	int iFlags;
};

struct lighting_ext
{
	Vector ambientlight;
	Vector diffuselight;
	Vector lightdir;
};

//========================================
//			PROP MANAGER DEFINITIONS
//
//========================================
constexpr int MAX_POINTS = 64;

//========================================
//			PROP MANAGER STRUCTS
//
//========================================
typedef struct epair_s
{
	struct epair_s* next;
	std::string key;
	std::string value;

	epair_s() : next(nullptr) {}

} epair_t;

typedef struct
{
	Vector origin;
	int firstbrush;
	int numbrushes;
	epair_t* epairs;
} entity_t;

struct vbomesh_t
{
	int start_vertex;
	int num_vertexes;
};

struct vbosubmodel_t
{
	vbomesh_t* meshes;
	int nummeshes;
};

struct vboheader_t
{
	brushvertex_t* pBufferData;
	int numverts;

	unsigned int* indexes;
	int numindexes;

	vbosubmodel_t* submodels;
	int numsubmodels;
};

struct modeldata_t
{
	std::string name;

	studiohdr_t* pHdr;
	studiohdr_t* pTexHdr;
	vboheader_t pVBOHeader;
};

struct entextradata_t
{
	Vector absmax;
	Vector absmin;
	Vector lightorigin;

	int num_leafs;
	short leafnums[MAX_ENT_LEAFS];
	float pbones[MAXSTUDIOBONES][3][4];

	modeldata_t* pModelData;
};

struct entextrainfo_t
{
	int surfindex;
	int lightstyles[4];
	Vector prevpos;
	int run_count = 0; // bacontsu - weird fucking workaround because static entities lightmap are shit the first time they take the value;

	lighting_ext pLighting;
	cl_entity_t* pEntity;
	entextradata_t* pExtraData; // only used by CL ents
};

struct cabledata_t
{
	int iwidth;
	int isegments;

	Vector vmins;
	Vector vmaxs;

	Vector vpoints[MAX_POINTS];
	int inumpoints;

	int num_leafs;
	short leafnums[MAX_ENT_LEAFS];
};

struct glstate_t
{
	glstate_t() : blending_enabled(false),
				  alphatest_enabled(false),
				  alphatest_func(0),
				  alphatest_value(0),
				  active_texunit(0),
				  active_clienttexunit(0)
	{
	}

	bool blending_enabled;

	bool alphatest_enabled;
	GLint alphatest_func;
	GLfloat alphatest_value;

	GLint active_texunit;
	GLint active_clienttexunit;
};

//========================================
//				LINUX DEFINITIONS
//
//========================================
#ifndef _WIN32
#define MessageBox(a, b, c, d) 0
#define MB_OK 0
void *wglGetProcAddress(const char *name);
#endif

//========================================
//				GLOBAL FUNCTION CALLS
//
//========================================
extern engine_studio_api_t IEngineStudio;

extern void ClampColor(int r, int g, int b, color24* out);
extern std::string FilenameFromPath(const std::string& inputPath);

extern void MyLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz);
extern mleaf_t* Mod_PointInLeaf(Vector p, model_t* model);
extern byte* Mod_LeafPVS(mleaf_t* leaf, model_t* model);
extern void R_MarkLeaves(mleaf_t* pLeaf);

extern void HUD_PrintSpeeds();
extern void RenderersDumpInfo();
extern void GenDetail();
extern void SetupFlashlight(Vector origin, Vector angles, float time, float frametime);
extern void ExportWorld();

extern unsigned short ByteToUShort(byte* byte);
extern unsigned int ByteToUInt(byte* byte);
extern int ByteToInt(byte* byte);

extern void R_CalcRefDef(ref_params_t* pparams);
extern void R_DrawNormalTriangles();
extern void R_DrawTransparentTriangles();

extern void RenderFog();
extern void BlackFog();
extern void DisableFog();
extern void ClearToFogColor();

extern void R_RotateForEntity(cl_entity_t* e);
extern int IsEntityMoved(cl_entity_t* e);
extern int IsEntityTransparent(cl_entity_t* e);
extern int IsPitchReversed(float pitch);
extern int TrinityBoxOnPlaneSide(Vector emins, Vector emaxs, mplane_t* p);

extern void DotProductSSE(float* result, const float* v0, const float* v1);
extern void SSEDotProductWorld(float* result, const float* v0, const float* v1);
extern void SSEDotProductWorldInt(int* result, const float* v0, const float* v1);
extern void SSEDotProductSub(float* result, Vector* v0, Vector* v1, float* subval);

extern void VectorAddSSE(const float* v0, const float* v1, const float* result);
extern void VectorMASSE(const float* veca, float scale, const float* vecb, float* vecc);
extern void VectorTransformSSE(const float* in1, float in2[3][4], float* out);
extern void VectorRotateSSE(const float* in1, float in2[3][4], float* out);
extern float VectorNormalizeFast(float* v);

extern void VectorRotate(const float* in1, const float in2[3][4], float* out);
extern void VectorIRotate(const Vector& in1, const float in2[3][4], Vector& out);
extern void FixVectorForSpotlight(Vector& vec);
extern void SV_FindTouchedLeafs(entextradata_t* ent, mnode_t* node);

extern void R_DisableSteamMSAA();
extern void R_SaveGLStates();
extern void R_RestoreGLStates();

extern void R_Init();
extern void R_VidInit();
extern void R_Shutdown();

// extern Vector	g_vecFull;
// extern Vector	g_vecZero;
constexpr int BASE_EXT_TEXTURE_ID = (1 << 25); // First GL texture id the renderer owns; engine ids stay below this
extern int current_ext_texture_id;
#endif

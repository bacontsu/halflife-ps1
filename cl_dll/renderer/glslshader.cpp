// FranticDreamer 2022-2025

#include "hud.h"
#include "cl_util.h"

#include "rendererdefs.h"
#include "glslshader.h"

// GL 2.0 shader entry points
static PFNGLCREATESHADERPROC pglCreateShader = nullptr;
static PFNGLDELETESHADERPROC pglDeleteShader = nullptr;
static PFNGLSHADERSOURCEPROC pglShaderSource = nullptr;
static PFNGLCOMPILESHADERPROC pglCompileShader = nullptr;
static PFNGLGETSHADERIVPROC pglGetShaderiv = nullptr;
static PFNGLGETSHADERINFOLOGPROC pglGetShaderInfoLog = nullptr;

static PFNGLCREATEPROGRAMPROC pglCreateProgram = nullptr;
static PFNGLDELETEPROGRAMPROC pglDeleteProgram = nullptr;
static PFNGLATTACHSHADERPROC pglAttachShader = nullptr;
static PFNGLDETACHSHADERPROC pglDetachShader = nullptr;
static PFNGLLINKPROGRAMPROC pglLinkProgram = nullptr;
static PFNGLGETPROGRAMIVPROC pglGetProgramiv = nullptr;
static PFNGLGETPROGRAMINFOLOGPROC pglGetProgramInfoLog = nullptr;
static PFNGLUSEPROGRAMPROC pglUseProgram = nullptr;

static PFNGLGETUNIFORMLOCATIONPROC pglGetUniformLocation = nullptr;
static PFNGLUNIFORM1IPROC pglUniform1i = nullptr;
static PFNGLUNIFORM1FPROC pglUniform1f = nullptr;
static PFNGLUNIFORM2FPROC pglUniform2f = nullptr;
static PFNGLUNIFORM3FPROC pglUniform3f = nullptr;
static PFNGLUNIFORM4FPROC pglUniform4f = nullptr;
static PFNGLUNIFORM4FVPROC pglUniform4fv = nullptr;

/*
====================
LoadFunctions

====================
*/
bool CGLSLShader::LoadFunctions()
{
	pglCreateShader = (PFNGLCREATESHADERPROC)wglGetProcAddress("glCreateShader");
	pglDeleteShader = (PFNGLDELETESHADERPROC)wglGetProcAddress("glDeleteShader");
	pglShaderSource = (PFNGLSHADERSOURCEPROC)wglGetProcAddress("glShaderSource");
	pglCompileShader = (PFNGLCOMPILESHADERPROC)wglGetProcAddress("glCompileShader");
	pglGetShaderiv = (PFNGLGETSHADERIVPROC)wglGetProcAddress("glGetShaderiv");
	pglGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC)wglGetProcAddress("glGetShaderInfoLog");

	pglCreateProgram = (PFNGLCREATEPROGRAMPROC)wglGetProcAddress("glCreateProgram");
	pglDeleteProgram = (PFNGLDELETEPROGRAMPROC)wglGetProcAddress("glDeleteProgram");
	pglAttachShader = (PFNGLATTACHSHADERPROC)wglGetProcAddress("glAttachShader");
	pglDetachShader = (PFNGLDETACHSHADERPROC)wglGetProcAddress("glDetachShader");
	pglLinkProgram = (PFNGLLINKPROGRAMPROC)wglGetProcAddress("glLinkProgram");
	pglGetProgramiv = (PFNGLGETPROGRAMIVPROC)wglGetProcAddress("glGetProgramiv");
	pglGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC)wglGetProcAddress("glGetProgramInfoLog");
	pglUseProgram = (PFNGLUSEPROGRAMPROC)wglGetProcAddress("glUseProgram");

	pglGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)wglGetProcAddress("glGetUniformLocation");
	pglUniform1i = (PFNGLUNIFORM1IPROC)wglGetProcAddress("glUniform1i");
	pglUniform1f = (PFNGLUNIFORM1FPROC)wglGetProcAddress("glUniform1f");
	pglUniform2f = (PFNGLUNIFORM2FPROC)wglGetProcAddress("glUniform2f");
	pglUniform3f = (PFNGLUNIFORM3FPROC)wglGetProcAddress("glUniform3f");
	pglUniform4f = (PFNGLUNIFORM4FPROC)wglGetProcAddress("glUniform4f");
	pglUniform4fv = (PFNGLUNIFORM4FVPROC)wglGetProcAddress("glUniform4fv");

	return pglCreateShader != nullptr && pglDeleteShader != nullptr && pglShaderSource != nullptr && pglCompileShader != nullptr && pglGetShaderiv != nullptr && pglGetShaderInfoLog != nullptr && pglCreateProgram != nullptr && pglDeleteProgram != nullptr && pglAttachShader != nullptr && pglDetachShader != nullptr && pglLinkProgram != nullptr && pglGetProgramiv != nullptr && pglGetProgramInfoLog != nullptr && pglUseProgram != nullptr && pglGetUniformLocation != nullptr && pglUniform1i != nullptr && pglUniform1f != nullptr && pglUniform2f != nullptr && pglUniform3f != nullptr && pglUniform4f != nullptr && pglUniform4fv != nullptr;
}

/*
====================
CompileStage

====================
*/
GLuint CGLSLShader::CompileStage(GLenum type, const char* pszSource, const char* pszName)
{
	GLuint uiShader = pglCreateShader(type);
	if (uiShader == 0)
		return 0;

	pglShaderSource(uiShader, 1, &pszSource, nullptr);
	pglCompileShader(uiShader);

	GLint iCompiled = 0;
	pglGetShaderiv(uiShader, GL_COMPILE_STATUS, &iCompiled);

	if (iCompiled == 0)
	{
		char szLog[1024];
		GLsizei iLength = 0;
		pglGetShaderInfoLog(uiShader, sizeof(szLog) - 1, &iLength, szLog);
		szLog[iLength] = '\0';

		gEngfuncs.Con_Printf("Failed to compile %s shader '%s':\n%s\n", (type == GL_VERTEX_SHADER) ? "vertex" : "fragment", pszName, szLog);

		pglDeleteShader(uiShader);
		return 0;
	}

	return uiShader;
}

/*
====================
CreateProgram

====================
*/
bool CGLSLShader::CreateProgram(const char* pszName, const char* pszVertexSource, const char* pszFragmentSource)
{
	Free();

	GLuint uiVertexShader = 0;
	GLuint uiFragmentShader = 0;

	if (pszVertexSource != nullptr)
	{
		uiVertexShader = CompileStage(GL_VERTEX_SHADER, pszVertexSource, pszName);
		if (uiVertexShader == 0)
			return false;
	}

	if (pszFragmentSource != nullptr)
	{
		uiFragmentShader = CompileStage(GL_FRAGMENT_SHADER, pszFragmentSource, pszName);
		if (uiFragmentShader == 0)
		{
			if (uiVertexShader != 0)
				pglDeleteShader(uiVertexShader);

			return false;
		}
	}

	m_uiProgram = pglCreateProgram();

	if (uiVertexShader != 0)
		pglAttachShader(m_uiProgram, uiVertexShader);

	if (uiFragmentShader != 0)
		pglAttachShader(m_uiProgram, uiFragmentShader);

	pglLinkProgram(m_uiProgram);

	// The program keeps the stages alive, flag them for deletion
	if (uiVertexShader != 0)
	{
		pglDetachShader(m_uiProgram, uiVertexShader);
		pglDeleteShader(uiVertexShader);
	}

	if (uiFragmentShader != 0)
	{
		pglDetachShader(m_uiProgram, uiFragmentShader);
		pglDeleteShader(uiFragmentShader);
	}

	GLint iLinked = 0;
	pglGetProgramiv(m_uiProgram, GL_LINK_STATUS, &iLinked);

	if (iLinked == 0)
	{
		char szLog[1024];
		GLsizei iLength = 0;
		pglGetProgramInfoLog(m_uiProgram, sizeof(szLog) - 1, &iLength, szLog);
		szLog[iLength] = '\0';

		gEngfuncs.Con_Printf("Failed to link shader program '%s':\n%s\n", pszName, szLog);

		pglDeleteProgram(m_uiProgram);
		m_uiProgram = 0;
		return false;
	}

	return true;
}

/*
====================
Free

====================
*/
void CGLSLShader::Free()
{
	if (m_uiProgram == 0)
		return;

	pglDeleteProgram(m_uiProgram);
	m_uiProgram = 0;
}

/*
====================
Bind

====================
*/
void CGLSLShader::Bind() const
{
	pglUseProgram(m_uiProgram);
}

/*
====================
Unbind

====================
*/
void CGLSLShader::Unbind()
{
	pglUseProgram(0);
}

/*
====================
GetUniform

====================
*/
GLint CGLSLShader::GetUniform(const char* pszName) const
{
	return pglGetUniformLocation(m_uiProgram, pszName);
}

void CGLSLShader::SetUniform1i(GLint location, int value) const
{
	pglUniform1i(location, value);
}

void CGLSLShader::SetUniform1f(GLint location, float x) const
{
	pglUniform1f(location, x);
}

void CGLSLShader::SetUniform2f(GLint location, float x, float y) const
{
	pglUniform2f(location, x, y);
}

void CGLSLShader::SetUniform3f(GLint location, float x, float y, float z) const
{
	pglUniform3f(location, x, y, z);
}

void CGLSLShader::SetUniform4f(GLint location, float x, float y, float z, float w) const
{
	pglUniform4f(location, x, y, z, w);
}

void CGLSLShader::SetUniform4fv(GLint location, int count, const float* pValues) const
{
	pglUniform4fv(location, count, pValues);
}

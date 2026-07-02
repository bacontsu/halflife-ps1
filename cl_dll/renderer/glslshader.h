// FranticDreamer 2022-2025

#if !defined(GLSLSHADER_H)
#define GLSLSHADER_H
#if defined(_WIN32)
#pragma once
#include "windows.h"
#endif

#include "GL/gl.h"
#include "gl/glext.h"


// Wrapper for a GL 2.0 programme.
// A programme can be built from a vertex shader, a fragment shader, or both. 
// Stages that are left out fall back to the fixed function pipeline.
class CGLSLShader
{
public:
	bool CreateProgram(const char* pszName, const char* pszVertexSource, const char* pszFragmentSource);
	void Free();

	void Bind() const;
	static void Unbind();

	bool IsValid() const { return m_uiProgram != 0; }
	GLint GetUniform(const char* pszName) const;

	void SetUniform1i(GLint location, int value) const;
	void SetUniform1f(GLint location, float x) const;
	void SetUniform2f(GLint location, float x, float y) const;
	void SetUniform3f(GLint location, float x, float y, float z) const;
	void SetUniform4f(GLint location, float x, float y, float z, float w) const;
	void SetUniform4fv(GLint location, int count, const float* pValues) const;

	// Loads the GL 2.0 entry points, returns false if any are missing
	static bool LoadFunctions();

private:
	static GLuint CompileStage(GLenum type, const char* pszSource, const char* pszName);

	GLuint m_uiProgram = 0;
};

#endif

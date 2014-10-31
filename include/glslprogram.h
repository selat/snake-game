#ifndef GLSL_PROGRAM_H
#define GLSL_PROGRAM_H

#include <GL/glew.h>

typedef struct GLSLProgram
{
	GLuint vertex_shader;
	GLuint fragment_shader;
	GLuint program;
} GLSLProgram;

void GLSL_Create(GLSLProgram **program, const char *vertex_shader, const char *fragment_shader);
void GLSL_Destroy(GLSLProgram **program);
void GLSL_Enable(GLSLProgram *program);

GLint GLSL_GetAttrib(GLSLProgram *program, const char *name);
GLint GLSL_GetUniform(GLSLProgram *program, const char *name);

#endif

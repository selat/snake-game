#include <stdio.h>
#include <stdlib.h>

#include <SDL2/SDL_rwops.h>

#include "glslprogram.h"

#define GLSLPROGRAM_LOG_SIZE 2048

static char glsl_log[GLSLPROGRAM_LOG_SIZE];

static GLuint createShader(GLenum type, const char *file_name);
static int linkProgram(GLSLProgram *program);

void GLSL_Create(GLSLProgram **program, const char *vertex_shader, const char *fragment_shader)
{
	*program = malloc(sizeof(GLSLProgram));
	(*program)->vertex_shader = createShader(GL_VERTEX_SHADER, vertex_shader);
	(*program)->fragment_shader = createShader(GL_FRAGMENT_SHADER, fragment_shader);
	(*program)->program = glCreateProgram();
	if(!linkProgram(*program)) {
		free(*program);
		*program = NULL;
	}
}

void GLSL_Destroy(GLSLProgram **program)
{
	glDeleteProgram((*program)->program);
	glDeleteShader((*program)->vertex_shader);
	glDeleteShader((*program)->fragment_shader);
	free(*program);
	*program = NULL;
}

void GLSL_Enable(GLSLProgram *program)
{
	glUseProgram(program->program);
}

GLint GLSL_GetAttrib(GLSLProgram *program, const char *name)
{
	return glGetAttribLocation(program->program, name);
}

GLint GLSL_GetUniform(GLSLProgram *program, const char *name)
{
	return glGetUniformLocation(program->program, name);
}

static GLuint createShader(GLenum type, const char *file_name)
{
	printf("Loading %-32s", file_name);
	GLuint shader = glCreateShader(type);
	
	SDL_RWops *file = SDL_RWFromFile(file_name, "r");
	if(file == NULL) {
		fprintf(stderr, "%s\n", SDL_GetError());
		return 0;
	}
	size_t file_size = SDL_RWsize(file);
	char *source = malloc(file_size + 1);

	SDL_RWread(file, source, file_size, file_size);
	SDL_RWclose(file);
	source[file_size] = '\0';

	glShaderSource(shader, 1, (const char**)(&source), NULL);
	glCompileShader(shader);

	GLint compile_ok = GL_FALSE;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compile_ok);
	if(compile_ok == GL_FALSE) {
		int length;
		glGetShaderInfoLog(shader, GLSLPROGRAM_LOG_SIZE, &length, glsl_log);
		fprintf(stderr, "failed!\n");
		glDeleteShader(shader);
		fprintf(stderr, "%s\n", glsl_log);
		return 0;
	}
	printf("ok!\n");
	return shader;
}

static int linkProgram(GLSLProgram *program)
{
	GLint status;
	glAttachShader(program->program, program->vertex_shader);
	glAttachShader(program->program, program->fragment_shader);
	glLinkProgram(program->program);
	glGetProgramiv(program->program, GL_LINK_STATUS, &status);
	if(status == GL_FALSE) {
		int length;
		glGetProgramInfoLog(program->program, GLSLPROGRAM_LOG_SIZE, &length, glsl_log);
		fprintf(stderr, "%s", glsl_log);
		return 0;
	}
	return 1;
}

#include <stdio.h>
#include <GL/glew.h>

#include "smm.h"
#include "glslprogram.h"

static GLint projection_matrix_id;
static GLint modelview_matrix_id;
static Matrix4x4 projection_matrix;
static Matrix4x4 modelview_matrix_stack[SMM_MATRIX_STACK_SIZE];
static int last_stack_matrix;

static void loadIdentity(Matrix4x4 *matrix);
static void copyMatrix(Matrix4x4 *src, Matrix4x4 *dst);

void SMM_Init(GLSLProgram *program, const char *projection_matrix_name, const char *modelview_matrix_name)
{
	modelview_matrix_id = GLSL_GetUniform(program, modelview_matrix_name);
	projection_matrix_id = GLSL_GetUniform(program, projection_matrix_name);
	loadIdentity(&projection_matrix);
	last_stack_matrix = 0;
	loadIdentity(modelview_matrix_stack + last_stack_matrix);
}

void SMM_Ortho(float left, float right, float bottom, float top, float znear, float zfar)
{
	projection_matrix.data[(0 << 2) + 0] = 2.0  / (right - left);
	projection_matrix.data[(1 << 2) + 1] = 2.0  / (top - bottom);
	projection_matrix.data[(2 << 2) + 2] = -2.0 / (zfar - znear);
	projection_matrix.data[(3 << 2) + 0] = -(right + left) / (right - left);
	projection_matrix.data[(3 << 2) + 1] = -(top + bottom) / (top - bottom);
	projection_matrix.data[(3 << 2) + 2] = -(zfar + znear) / (zfar - znear);
}

void SMM_Translate(float x, float y, float z)
{
	float *data = modelview_matrix_stack[last_stack_matrix].data;
	data[(3 << 2) + 0] += x;
	data[(3 << 2) + 1] += y;
	data[(3 << 2) + 2] += z;
}

void SMM_Push()
{
	copyMatrix(modelview_matrix_stack + last_stack_matrix, modelview_matrix_stack + last_stack_matrix + 1);
	++last_stack_matrix;
}

void SMM_Pop()
{
	--last_stack_matrix;
}

void SMM_LoadIdentity()
{
	loadIdentity(modelview_matrix_stack + last_stack_matrix);
}

void SMM_LoadGL()
{
	glUniformMatrix4fv(projection_matrix_id, 1, GL_FALSE, projection_matrix.data);
	glUniformMatrix4fv(modelview_matrix_id, 1, GL_FALSE, modelview_matrix_stack[last_stack_matrix].data);
}

static void loadIdentity(Matrix4x4 *matrix)
{
	int i;
	for(i = 0; i < 16; ++i) {
		matrix->data[i] = 0.0f;
	}
	matrix->data[(0 << 2) + 0] = 1.0f;
	matrix->data[(1 << 2) + 1] = 1.0f;
	matrix->data[(2 << 2) + 2] = 1.0f;
	matrix->data[(3 << 2) + 3] = 1.0f;
}

static void copyMatrix(Matrix4x4 *src, Matrix4x4 *dst)
{
	int i;
	for(i = 0; i < 16; ++i) {
		dst->data[i] = src->data[i];
	}
}

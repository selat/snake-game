#ifndef MATRIX_H
#define MATRIX_H

/*
  Scene Matrix Manager
 */

#define SMM_MATRIX_STACK_SIZE 16

typedef struct Matrix4x4
{
	float data[16];
} Matrix4x4;

struct GLSLProgram;

void SMM_Init(struct GLSLProgram *program, const char *projection_matrix_name, const char *modelview_matrix_name);
void SMM_Ortho(float left, float right, float bottom, float top, float znear, float z_far);
void SMM_Translate(float x, float y, float z);
void SMM_Push();
void SMM_Pop();
void SMM_LoadIdentity();
void SMM_LoadGL();

#endif

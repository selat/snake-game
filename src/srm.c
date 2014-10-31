#include <SDL2/SDL.h>
#include <GL/glew.h>

#include "glslprogram.h"
#include "scene.h"
#include "smm.h"
#include "srm.h"

#define CELL_TYPES_NUM (sizeof(colors) / sizeof(colors[0]) / 4 * 3)

SDL_Window *window;
SDL_GLContext gl_context;

GLSLProgram *program;

GLfloat colors[] = {
	/* Empty cell */
	0.5, 0.5, 0.0, 1.0,
	/* Snake head */
	0.0, 0.5, 0.5, 1.0,
	/* Dead snake head */
	0.8, 0.0, 0.0, 1.0,
	/* Snake body */
	0.5, 0.0, 0.5, 1.0,
	/* Apple */
	0.0, 0.7, 0.0, 1.0,
};

GLuint vbo_cell;
GLuint vbo_colors[CELL_TYPES_NUM];

GLint attrib_coord;
GLint attrib_color;

int width;
int height;

static void createColorBuffers();
static void createCellBuffer();
static void renderCell(int id, int x, int y);
static void renderCellf(int id, float x, float y);
static void renderSnake(Scene *scene);
static void renderItems(Scene *scene);

int SRM_Init()
{
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	window = SDL_CreateWindow("Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480,
//	                          SDL_WINDOW_FULLSCREEN_DESKTOP |
	                          SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(window == NULL) {
		fprintf(stderr, "%s", SDL_GetError());
		return 0;
	}
	SDL_GetWindowSize(window, &width, &height);
	gl_context = SDL_GL_CreateContext(window);

	GLenum status = glewInit();
	if(status != GLEW_OK) {
		fprintf(stderr, "%s", (const char*)(glewGetErrorString(status)));
		return 0;
	}

//	resize(width, height);
	glEnable(GL_CULL_FACE);
	glEnable(GL_POLYGON_SMOOTH);
	glDisable(GL_DEPTH_TEST);
	glCullFace(GL_BACK);

	GLSL_Create(&program, "color.vert", "color.frag");
	GLSL_Enable(program);

	attrib_coord = GLSL_GetAttrib(program, "coord");
	attrib_color = GLSL_GetAttrib(program, "color_f");

	SMM_Init(program, "projection_matrix", "modelview_matrix");
	SMM_Ortho(0.0, (float)SCENE_WIDTH, 0.0f, (float)SCENE_HEIGHT, -1.0f, 1.0f);

	createColorBuffers();
	createCellBuffer();

	return 1;
}

void SRM_Render(Scene *scene)
{
	int x, y, id;
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	SMM_LoadIdentity();

	glEnableVertexAttribArray(attrib_coord);
	glEnableVertexAttribArray(attrib_color);
	for(x = 0; x < SCENE_WIDTH; ++x) {
		for(y = 0; y < SCENE_HEIGHT; ++y) {
			id = SCENE_GetCell(scene, x, y);
			renderCell(id, x, y);
		}
	}
	renderItems(scene);
	renderSnake(scene);

	glDisableVertexAttribArray(attrib_coord);
	glDisableVertexAttribArray(attrib_color);

	SDL_GL_SwapWindow(window);
}

void SRM_Quit()
{
	GLSL_Destroy(&program);
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
}

static void createColorBuffers()
{
	int i, j;
	static GLfloat tmp_colors[16];
	glGenBuffers(CELL_TYPES_NUM, vbo_colors);
	for(i = 0; i < CELL_TYPES_NUM; ++i) {
		for(j = 0; j < 4; ++j) {
			tmp_colors[(j << 2) + 0] = colors[(i << 2) + 0];
			tmp_colors[(j << 2) + 1] = colors[(i << 2) + 1];
			tmp_colors[(j << 2) + 2] = colors[(i << 2) + 2];
			tmp_colors[(j << 2) + 3] = colors[(i << 2) + 3];
		}

		glBindBuffer(GL_ARRAY_BUFFER, vbo_colors[i]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 4 * 4, tmp_colors, GL_STATIC_DRAW);
	}
}

static void createCellBuffer()
{
	static GLfloat coords[8] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f,
	};

	glGenBuffers(1, &vbo_cell);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cell);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 2 * 4, coords, GL_STATIC_DRAW);
}

static void renderCellf(int id, float x, float y)
{
	SMM_Push();
	SMM_Translate(x, y, 0.0f);
	SMM_LoadGL();
	glBindBuffer(GL_ARRAY_BUFFER, vbo_cell);
	glVertexAttribPointer(attrib_coord, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_colors[id]);
	glVertexAttribPointer(attrib_color, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_QUADS, 0, 4);
	SMM_Pop();
}

static void renderCell(int id, int x, int y)
{
	renderCellf(id, (float)x, (float)y);
}

static void renderSnake(Scene *scene)
{
	int i, x, y;
	float shiftx, shifty;
	int head_cell = 1;
	if(scene->is_game_over) {
		head_cell = 2;
	}
	if(scene->snake_length > 1) {
		renderCellf(3, scene->snake[0][0], scene->snake[0][1]);
	}
	for(i = 1; i < scene->snake_length - 1; ++i) {
		renderCellf(3, scene->snake[i][0], scene->snake[i][1]);
	}
	/* If snake is growing, it's tail shouldn't move */
	if((scene->snake_length > 1) && !scene->is_growing) {
		SCENE_GetSnakeLastShift(scene, &shiftx, &shifty);
		renderCellf(3, scene->snake[scene->snake_length - 1][0] + shiftx, scene->snake[scene->snake_length - 1][1] + shifty);
		SCENE_GetNextPos(scene, 0, &x, &y);
	}
	SCENE_GetSnakeShift(scene, &shiftx, &shifty);
	SCENE_GetNextPos(scene, 0, &x, &y);
	renderCellf(head_cell, scene->snake[0][0] + shiftx, scene->snake[0][1] + shifty);
	switch(scene->last_move_id) {
	case 0:
		renderCellf(head_cell, x - 1.0 + shiftx, y);
		break;
	case 1:
		renderCellf(head_cell, x, y + 1.0 + shifty);
		break;
	case 2:
		renderCellf(head_cell, x + 1.0 + shiftx, y);
		break;
	case 3:
		renderCellf(head_cell, x, y - 1.0 + shifty);
		break;
	}
}

static void renderItems(Scene *scene)
{
	int i;
	for(i = 0; i < scene->items_num; ++i) {
		renderCell(4, scene->items[i][0], scene->items[i][1]);
	}
}

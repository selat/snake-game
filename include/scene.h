#ifndef SCENE_H
#define SCENE_H

#include <inttypes.h>

#define SCENE_WIDTH 16
#define SCENE_HEIGHT 16
#define SCENE_MAX_SNAKE_LENGTH 1000
/* 1 tick ~ 1 millisecond */
#define SCENE_TICKS_PER_STEP 100
#define SCENE_MAX_ITEMS 10

typedef struct Scene
{
	int field[SCENE_WIDTH * SCENE_HEIGHT];
	/* First point - head, last - tail*/
	int snake[SCENE_MAX_SNAKE_LENGTH][2];
	int snake_length;

	int items_num;
	int items[SCENE_MAX_ITEMS][2];

	int last_dir[2];
	int is_growing;
	int is_game_over;

	int move_id, last_move_id;
	uint32_t last_update_time;
	
} Scene;

struct SDL_KeyboardEvent;

void SCENE_Init(Scene *scene);
void SCENE_Update(Scene *scene);
void SCENE_KeyDown(Scene *scene, struct SDL_KeyboardEvent *e);
int SCENE_GetCell(Scene *scene, int x, int y);
void SCENE_SetCell(Scene *scene, int x, int y, int val);
void SCENE_GetSnakeShift(Scene *scene, float *shiftx, float *shifty);
void SCENE_GetSnakeLastShift(Scene *scene, float *shiftx, float *shifty);
void SCENE_GetNextPos(Scene *scene, int part_id, int *x, int *y);

#endif

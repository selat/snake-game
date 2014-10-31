#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL.h>

#include "scene.h"

static int move_directions[4][2] = {
	{1, 0},
	{0, -1},
	{-1, 0},
	{0, 1}
};

static void placeRandomItem(Scene *scene);
/* Try to eat items, return 1 on success, 0 on failure. */
static int eatItems(Scene *scene);
static int hitObstacle(Scene *scene);

void SCENE_Init(Scene *scene)
{
	int i, j;
	for(i = 0; i < SCENE_WIDTH; ++i) {
		for(j = 0; j < SCENE_HEIGHT; ++j) {
			SCENE_SetCell(scene, i, j, 0);
		}
	}
	scene->snake_length = 1;
	scene->snake[0][0] = SCENE_WIDTH / 2;
	scene->snake[0][1] = SCENE_HEIGHT / 2;
	scene->items_num = 0;
	scene->move_id = 3;
	scene->last_move_id = 3;
	scene->last_update_time = SDL_GetTicks();
	scene->is_growing = 0;
	scene->is_game_over = 0;
	scene->last_dir[0] = 0;
	scene->last_dir[1] = 0;
}

void SCENE_Update(Scene *scene)
{
	if(hitObstacle(scene)) {
		scene->is_game_over = 1;
	}
	if((SDL_GetTicks() > scene->last_update_time + SCENE_TICKS_PER_STEP) && !scene->is_game_over) {
		int i, lastx, lasty, len;
		len = scene->snake_length;
		/* This works only if position of head wasn't changed yet. */
		for(i = len - 1; i > 0; --i) {
			scene->snake[i][0] = scene->snake[i - 1][0];
			scene->snake[i][1] = scene->snake[i - 1][1];
		}
		lastx = scene->snake[len - 1][0];
		lasty = scene->snake[len - 1][1];
		scene->snake[0][0] += move_directions[scene->last_move_id][0];
		scene->snake[0][1] += move_directions[scene->last_move_id][1];
		scene->last_move_id = scene->move_id;
		if(len > 1) {
			scene->last_dir[0] = scene->snake[len - 2][0] - scene->snake[len - 1][0];
			if(scene->last_dir[0] > 1) {
				scene->last_dir[0] = -1;
			} else if(scene->last_dir[0] < -1) {
				scene->last_dir[0] = 1;
			}
			scene->last_dir[1] = scene->snake[len - 2][1] - scene->snake[len - 1][1];
			if(scene->last_dir[1] > 1) {
				scene->last_dir[1] = -1;
			} else if(scene->last_dir[1] < -1) {
				scene->last_dir[1] = 1;
			}
		}
		/* Prevent snake from getting of the field. */
		for(i = 0; i < len; ++i) {
			scene->snake[i][0] = (scene->snake[i][0] + SCENE_WIDTH) % SCENE_WIDTH;
			scene->snake[i][1] = (scene->snake[i][1] + SCENE_HEIGHT) % SCENE_HEIGHT;
		}
		if(eatItems(scene) && (len < SCENE_MAX_SNAKE_LENGTH)) {
			scene->is_growing = 1;
			scene->snake[len][0] = lastx;
			scene->snake[len][1] = lasty;
			++scene->snake_length;
		} else {
			scene->is_growing = 0;
		}
		scene->last_update_time = SDL_GetTicks();
	}
	/* We update items after the snake movement to prevent generating items near the snake head (i.e. without rendering this item) */
	if((scene->items_num < SCENE_MAX_ITEMS) && !scene->is_game_over) {
		placeRandomItem(scene);
	}
}

void SCENE_KeyDown(Scene *scene, struct SDL_KeyboardEvent *e)
{
	switch(e->keysym.sym) {
	case SDLK_RIGHT:
		/* Snake can't rotate for 180 degrees for one turn */
		if(scene->last_move_id != 2) {
			scene->move_id = 0;
		}
		break;
	case SDLK_DOWN:
		if(scene->last_move_id != 3) {
			scene->move_id = 1;
		}
		break;
	case SDLK_LEFT:
		if(scene->last_move_id != 0) {
			scene->move_id = 2;
		}
		break;
	case SDLK_UP:
		if(scene->last_move_id != 1) {
			scene->move_id = 3;
		}
		break;
	}
}

int SCENE_GetCell(Scene *scene, int x, int y)
{
	return scene->field[x * SCENE_WIDTH + y];
}

void SCENE_SetCell(Scene *scene, int x, int y, int val)
{
	scene->field[x * SCENE_WIDTH + y] = val;
}


void SCENE_GetSnakeShift(Scene *scene, float *shiftx, float *shifty)
{
	uint32_t delta_time = SDL_GetTicks() - scene->last_update_time;
	*shiftx = move_directions[scene->last_move_id][0] * (float)delta_time / SCENE_TICKS_PER_STEP;
	if(*shiftx < 0.0f) {
		*shiftx = fmax(-1.0f, *shiftx);
	} else {
		*shiftx = fmin(1.0f, *shiftx);
	}

	*shifty = move_directions[scene->last_move_id][1] * (float)delta_time / SCENE_TICKS_PER_STEP;
	if(*shifty < 0.0f) {
		*shifty = fmax(-1.0f, *shifty);
	} else {
		*shifty = fmin(1.0f, *shifty);
	}
}

void SCENE_GetSnakeLastShift(Scene *scene, float *shiftx, float *shifty)
{
	uint32_t delta_time = SDL_GetTicks() - scene->last_update_time;
	*shiftx = scene->last_dir[0] * (float)delta_time / SCENE_TICKS_PER_STEP;
	if(scene->last_dir[0] < 0.0f) {
		*shiftx = fmax(-1.0f, *shiftx);
	} else {
		*shiftx = fmin(1.0f, *shiftx);
	}

	*shifty = scene->last_dir[1] * (float)delta_time / SCENE_TICKS_PER_STEP;
	if(scene->last_dir[1] < 0.0f) {
		*shifty = fmax(-1.0f, *shifty);
	} else {
		*shifty = fmin(1.0f, *shifty);
	}
}

void SCENE_GetNextPos(Scene *scene, int part_id, int *x, int *y)
{
	if(part_id == 0) {
		*x = (scene->snake[0][0] + move_directions[scene->last_move_id][0] + SCENE_WIDTH) % SCENE_WIDTH;
		*y = (scene->snake[0][1] + move_directions[scene->last_move_id][1] + SCENE_HEIGHT) % SCENE_HEIGHT;
	} else {
		*x = (scene->snake[part_id - 1][0] + SCENE_WIDTH) % SCENE_WIDTH;
		*y = (scene->snake[part_id - 1][1] + SCENE_HEIGHT) % SCENE_HEIGHT;
	}
}

static void placeRandomItem(Scene *scene)
{
	/* Algorithm (there are some problems with statistical distribution):
	 * 1. Mark all not empty cells
	 * 2. Select random row
	 * 3. Select random column
	 */
	static int free_cells[SCENE_WIDTH + SCENE_HEIGHT];
	static int free_cells_num = 0;
	int mark = (1 << 7), i, j, itemx, itemy;
	for(i = 0; i < scene->snake_length; ++i) {
		int id = SCENE_GetCell(scene, scene->snake[i][0], scene->snake[i][1]);
		SCENE_SetCell(scene, scene->snake[i][0], scene->snake[i][1], id | mark);
	}

	/* Select row */
	for(i = 0; i < SCENE_HEIGHT; ++i) {
		int have_free = 0;
		for(j = 0; j < SCENE_WIDTH; ++j) {
			int id = SCENE_GetCell(scene, j, i);
			if(!(id & mark)) {
				have_free = 1;
				break;
			}
		}
		if(have_free) {
			free_cells[free_cells_num++] = i;
		}
	}
	itemy = free_cells[rand() % free_cells_num];
	free_cells_num = 0;

	/* Select column */
	for(i = 0; i < SCENE_WIDTH; ++i) {
		int id = SCENE_GetCell(scene, i, itemy);
		if(!(id & mark)) {
			free_cells[free_cells_num++] = i;
		}
	}
	itemx = free_cells[rand() % free_cells_num];

	/* Clear mark */
	for(i = 0; i < scene->snake_length; ++i) {
		int id = SCENE_GetCell(scene, scene->snake[i][0], scene->snake[i][1]);
		SCENE_SetCell(scene, scene->snake[i][0], scene->snake[i][1], id & ~mark);
	}

	scene->items[scene->items_num][0] = itemx;
	scene->items[scene->items_num++][1] = itemy;
}

static int eatItems(Scene *scene)
{
	int i, j;
	for(i = 0; i < scene->items_num; ++i) {
		if((scene->snake[0][0] == scene->items[i][0]) &&
		   (scene->snake[0][1] == scene->items[i][1])) {
			/* Shift items */
			for(j = i; j < scene->items_num - 1; ++j) {
				scene->items[j][0] = scene->items[j + 1][0];
				scene->items[j][1] = scene->items[j + 1][1];
			}
			--scene->items_num;
			return 1;
		}
	}
	return 0;
}

static int hitObstacle(Scene *scene)
{
	int i, x, y;
	x = scene->snake[0][0] + move_directions[scene->last_move_id][0];
	y = scene->snake[0][1] + move_directions[scene->last_move_id][1];
	for(i = 1; i < scene->snake_length; ++i) {
		if((x == scene->snake[i][0]) && (y == scene->snake[i][1])) {
			return 1;
		}
	}
	return 0;
}

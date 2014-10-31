#include <SDL2/SDL.h>

#include "srm.h"
#include "scene.h"

Scene scene;

int init(void)
{
	SDL_Init(SDL_INIT_VIDEO);
	return 1;
}

int main(int argc, char **argv)
{
	if(!init()) {
		return -1;
	}

	if(!SRM_Init()) {
		return -1;
	}

	SCENE_Init(&scene);

	int running = 1;
	while(running) {
		SDL_Event e;
		while(SDL_PollEvent(&e)) {
			switch(e.type) {
			case SDL_QUIT:
				running = 0;
				break;
			case SDL_KEYDOWN:
				SCENE_KeyDown(&scene, &e.key);
				break;
			}
		}

		SCENE_Update(&scene);
		SRM_Render(&scene);
	}

	SRM_Quit();

	return 0;
}

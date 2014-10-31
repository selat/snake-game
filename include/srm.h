#ifndef SRM_H
#define SRM_H

/*
  Scene Rendering Manager
 */

struct Scene;

int SRM_Init();
void SRM_Quit();
void SRM_Render(struct Scene *scene);

#endif

/* hs.h - high score table */
#include "common.h"

void iniths();
void destroyhs();
void drawhs();
int handlehs();

extern SDL_Surface *screen, *numbers;
extern int gamestate;

extern SDL_Surface *tempsurf;

extern int sfxon, muson;

extern SDL_Joystick *joy;
extern int joysticks;

extern int arcade_mode;
extern SDLKey keys[10];
Uint32 hs_arcade_ticks;


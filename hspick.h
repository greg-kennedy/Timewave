/* hspick.h - high score colorpick */
#include "common.h"

void inithspick();
void destroyhspick();
void drawhspick();
int handlehspick();

extern SDL_Surface *screen;
extern int gamestate;

extern SDL_Surface *tempsurf;
SDL_Surface *mcursy;

extern int sfxon, muson;
extern Mix_Music *music;

extern long score;

extern SDL_Joystick *joy;
extern int joysticks;

extern SDLKey keys[10];

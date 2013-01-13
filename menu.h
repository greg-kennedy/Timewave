/* menu.h - menu */
#include "common.h"

void initmenu();
void destroymenu();
void drawmenu();
int handlemenu();

extern SDL_Surface *screen;
extern int gamestate;

extern SDL_Surface *tempsurf;

extern int sfxon, muson;
extern Mix_Music *music;


SDL_Surface *checkbox, *mcurs;

SDL_Rect cboxrect, mcrect;

extern SDL_Joystick *joy;
extern int joysticks;

extern SDLKey keys[10];

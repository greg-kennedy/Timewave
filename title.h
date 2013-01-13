/* title.h - title screen */
#include "common.h"

void inittitle();
void destroytitle();
void drawtitle();
int handletitle();

extern SDL_Surface *screen;
extern int gamestate, muson;

extern SDL_Surface *tempsurf;
extern Mix_Music *music;   /* SDL_Mixer stuff here */

extern SDL_Joystick *joy;
extern int joysticks;

extern int arcade_mode;
extern SDLKey keys[10];
Uint32 tscreen_arcade_ticks;

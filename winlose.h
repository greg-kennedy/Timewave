/* hs.h - high score table */
#include "common.h"

void initgameover();
void destroygameover();
void drawgameover();
int handlegameover();

void initvictory();
void destroyvictory();
void drawvictory();
int handlevictory();

extern SDL_Surface *screen, *numbers;
extern int gamestate;

extern SDL_Surface *tempsurf;

extern int sfxon, muson;
extern Mix_Music *music;   /* SDL_Mixer stuff here */

extern long score;
extern int tscore[3];

extern SDL_Joystick *joy;
extern int joysticks;

extern SDLKey keys[10];

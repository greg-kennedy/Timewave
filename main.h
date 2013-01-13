/* main.h - main header */
#include "common.h"

SDL_Surface *screen = NULL;
SDL_Surface *numbers = NULL;
int gamestate=0;
int muson=1, sfxon=1;
long score=0;

SDL_Surface *tempsurf = NULL;
Mix_Music *music = NULL;   /* SDL_Mixer stuff here */

int tscore[3]={0};
int red[3]={0};
int green[3]={0};
int blue[3]={0};

extern SDL_Joystick *joy;
extern int joysticks;

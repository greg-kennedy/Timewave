/* keyconf.h - keyconfig */
#include "common.h"

void initkeyconf();
void destroykeyconf();
void drawkeyconf();
int handlekeyconf();

extern SDL_Surface *screen;
extern int gamestate;

extern SDL_Surface *tempsurf;

extern SDLKey keys[10];

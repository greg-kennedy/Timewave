/* game.h - gameplay header */
#include "common.h"

void musicfinished();
void initgame(int);
void destroygame();
void spawn(int);
void draw();
int handlegame();
int updategame();

int createbullet(float, float, int, int, float);
int createenemy(float, float, int, float, float,int);
int createexplosion(float, float, int);

void playnoise(int);

extern SDL_Surface *screen, *numbers;
extern int gamestate;
extern long score;

struct bullet
{
       float x;
       float y;
       int theta;
       float speed;
       int type;
};

struct enemy
{
       float x;
       float y;
       int type;
       float arg1;
       float arg2;
       float arg3;
       int life;
};

struct spawninfo
{
       int when;
       float x;
       float y;
       int type;
       float arg1;
       float arg2;
       int life;
};

struct explosion
{
       float x;
       float y;
       float img;
       int type;
};

extern Mix_Music *music;
extern int muson;
extern int sfxon;

extern SDL_Joystick *joy;
extern int joysticks;

extern SDLKey keys[10];

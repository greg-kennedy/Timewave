/* game.c - gameplay */

#include "game.h"

#include <math.h>

#include "common.h"

#include "meta_play.h"

#define SCROLLSPEED 4
#define EXPLOSPEED 4
//#define SHIPSPEED 1.5

/* num imgs to load */
#define MAXSHIPS 4
#define MAXBULS 5
#define MAXENS 255
#define MAXEXP 5

#define PI 3.14159265358

/* max num on screen */
#define SCREENBUL 500
#define SCREENE 32
#define ALLSPAWN 128
#define SCREENEX 16

/* Sample names */
#define SLOW 0
#define NORMAL 1
#define FAST 2

#define P_DIE 0
#define E_DIE 1
#define B_DIE 2
#define P_SHOT 3
#define E_SHOT 4

#define S_UP 5
#define S_DOWN 6

/* ************************************************************** */
// STRUCT DEFINITIONS
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

/* ************************************************************** */
// OBJECT POOLS
static struct bullet bullets[SCREENBUL];
static struct enemy elist[SCREENE];
static struct explosion explo[SCREENEX];

/* ************************************************************** */
// PRELOADED Resources
static Mix_Chunk* chunk_up;
static Mix_Chunk* chunk_down;
static Mix_Chunk* chunk_sfx[3][5];

static Mix_Music* music_boss;

static SDL_Surface* bg;
static SDL_Surface* img_pause;
static SDL_Surface* img_overlay[3];
static SDL_Surface* img_player[MAXSHIPS];
static SDL_Surface* img_shot[MAXBULS];
static SDL_Surface* img_enemy[MAXENS];
static SDL_Surface* img_explo[MAXEXP*5];

static float sintbl[360];

// TODO: this is bad
//float rate;

/* ************************************************************** */
// SHARED GAME DATA INIT / PRELOAD
int init_game(struct env_t* env)
{
	char buffer[30];

	for (int i = 0;i<360;i++)
		sintbl[i] = sin(PI * (float)i / 180);

	/* LOAD ALL IMAGES */
	bg = load_image("img/gfx/bg.png", 0);
	img_pause = load_image("img/ui/paused.png", 1);

	for (int i = 0;i<3;i++)
	{
		sprintf(buffer, "img/gfx/top%d.png", i);
		img_overlay[i] = load_image(buffer, 1);
	}

	for (int i = 0;i<MAXSHIPS;i++)
	{
		sprintf(buffer, "img/gfx/ship%d.png", i);
		img_player[i] = load_image(buffer, 1);
	}

	for (int i = 0;i<MAXENS;i++)
	{
		sprintf(buffer, "img/gfx/en%d.png", i);
		img_enemy[i] = load_image(buffer, 1);
	}

	for (int i = 0;i<MAXEXP*5;i++)
	{
		sprintf(buffer, "img/gfx/exp%d.png", i);
		img_explo[i] = load_image(buffer, 1);
	}

	for (int i = 0;i<MAXBULS;i++)
	{
		sprintf(buffer, "img/gfx/bul%d.png", i);
		img_shot[i] = load_image(buffer, 1);
	}

	/* Sound FX */
	if (env->sfx_works && env->sfx_on) {
		chunk_up = load_sample("audio/sup.wav");
		chunk_down = load_sample("audio/sdn.wav");
		chunk_sfx[SLOW][P_DIE] = load_sample("audio/sex2.wav");
		chunk_sfx[SLOW][E_DIE] = load_sample("audio/sex1.wav");
		chunk_sfx[SLOW][B_DIE] = load_sample("audio/sex3.wav");
		chunk_sfx[SLOW][P_SHOT] = load_sample("audio/sbul2.wav");
		chunk_sfx[SLOW][E_SHOT] = load_sample("audio/sbul1.wav");
		chunk_sfx[NORMAL][P_DIE] = load_sample("audio/ex2.wav");
		chunk_sfx[NORMAL][E_DIE] = load_sample("audio/ex1.wav");
		chunk_sfx[NORMAL][B_DIE] = load_sample("audio/ex3.wav");
		chunk_sfx[NORMAL][P_SHOT] = load_sample("audio/bul2.wav");
		chunk_sfx[NORMAL][E_SHOT] = load_sample("audio/bul1.wav");
		chunk_sfx[FAST][P_DIE] = load_sample("audio/fex2.wav");
		chunk_sfx[FAST][E_DIE] = load_sample("audio/fex1.wav");
		chunk_sfx[FAST][B_DIE] = load_sample("audio/fex3.wav");
		chunk_sfx[FAST][P_SHOT] = load_sample("audio/fbul2.wav");
		chunk_sfx[FAST][E_SHOT] = load_sample("audio/fbul1.wav");
	}

	// BOSS music
	if (env->mus_works && env->mus_on)
		music_boss = load_music("audio/boss.mod", 0);
	
	return 0;
}

/* ************************************************************** */
// SHARED GAME FREE
void free_game()
{
	/* Images */
	SDL_FreeSurface(bg);
	SDL_FreeSurface(img_pause);

	for (int i = 0;i<MAXSHIPS;i++)
		SDL_FreeSurface(img_player[i]);

	for (int i = 0;i<MAXEXP*5;i++)
		SDL_FreeSurface(img_explo[i]);

	for (int i = 0;i<3;i++)
		SDL_FreeSurface(img_overlay[i]);

	for (int i = 0;i<MAXBULS;i++)
		SDL_FreeSurface(img_shot[i]);

	for (int i = 0;i<MAXENS;i++)
		SDL_FreeSurface(img_enemy[i]);

	// Sound FX
	Mix_FreeChunk(chunk_up);
	Mix_FreeChunk(chunk_down);
	for (int i = 0; i < 3; i ++)
		for (int j = 0; j < 5; j ++)
			Mix_FreeChunk(chunk_sfx[i][j]);

	// Music
	Mix_FreeMusic(music_boss);
}

// Trigger a sound effect, with panning.
static void playnoise(const int sample, const int rate, const int x)
{
	Mix_Chunk* chunk;

	if (sample == 5) chunk = chunk_up;
	else if (sample == 6) chunk = chunk_down;
	else chunk = chunk_sfx[rate][sample];

	if (chunk != NULL)
	{
		// TODO: possibly kill oldest channel to make room for a new sample
		int channel = Mix_PlayChannel(-1, chunk, 0);
		if (channel >= 0)
			Mix_SetPanning(channel, x, 254 - x);
	}
}

// Add a bullet to the bullet object pool
static int createbullet(const float x, const float y, const int type, const int theta, const float speed)
{
	int i;
	for(i = 0;i<SCREENBUL;i++)
		if (bullets[i].type == 0)
		{
			bullets[i].type = type;
			bullets[i].x = x;
			bullets[i].y = y;
			bullets[i].theta = theta;
			bullets[i].speed = speed;
			break;
		}

	return (i == SCREENBUL);
}

// Create an explosion in the explosion pool
static int createexplosion(const float x, const float y, const int type)
{
	int i;
	for(i = 0;i<SCREENEX;i++)
		if (explo[i].type == 0)
		{
			explo[i].type = type;
			explo[i].x = x;
			explo[i].y = y;
			explo[i].img = 0;
			break;
		}
	return (i == SCREENEX);
}

// Create an enemy in the enemy pool
static int createenemy(float x, float y, int type, float arg1, float arg2, int life)
{
	int i;
	for(i = 0;i<SCREENE;i++)
		if (elist[i].type == 0)
		{
			elist[i].type = type;
			elist[i].x = x;
			elist[i].y = y;
			elist[i].arg1 = arg1;
			elist[i].arg2 = arg2;
			elist[i].arg3 = 0;
			elist[i].life = life;
			break;
		}
	return (i == SCREENE);
}

/* ************************************************************** */
// MAIN GAME LOOP
int state_game(struct env_t* env, Mix_Music* music_pause, const int level, int* score)
{
	int i;
	FILE* fp;

	// enemy point values
	const long scores[256] =
	{0, 100, 50, 100, 250, 50, 500, 0, 500, 100,
		50, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 100000, 100000, 100000, 10, 10, 10, 0, 10000,
		50000, 30000, 40000, 20000, 100000, 100000};

	/* ************************************************************** */
	// NON-PRELOADED Resources
	struct spawninfo slist[ALLSPAWN];
	SDL_Surface* strips[42] = {NULL};
	int stripslist[128] = {0};

	/* LOAD DATA FILES FOR LEVEL */
	char buffer[30];

	sprintf(buffer, "data/%d.map", level);
	fp = fopen(buffer, "rb");
	for(i = 0;i<128;i++)
		fread(&stripslist[i], 1, 1, fp);
	fclose(fp);

	sprintf(buffer, "data/%d.ens", level);
	fp = fopen(buffer, "r");
	for(i = 0;i<ALLSPAWN;i++)
		if (!feof(fp))
			fscanf(fp, "%d %f %f %d %f %f %d\n", &slist[i].when, &slist[i].x, &slist[i].y, &slist[i].type, &slist[i].arg1, &slist[i].arg2, &slist[i].life);
		else
			slist[i].when = 999;
	fclose(fp);

	Mix_Music* music = NULL;
	if (env->mus_works && env->mus_on)
	{
		sprintf(buffer, "audio/%d.mod", level);
		music = load_music(buffer, 1);
	}

	if (level<6) {
		for (i = 1;i<42;i++)
		{
			sprintf(buffer, "img/strips/%d/%d.png", level, i);
			strips[i] = load_image(buffer, 1);
		}
	}

	/* setup rectangles */
	SDL_Rect striprect;
	striprect.x = 0;
	striprect.w = 800;
	striprect.h = 128;

	SDL_Rect bgrect;
	bgrect.x = 0;
	bgrect.w = 800;
	bgrect.h = 600;

	SDL_Rect shiprect;
	shiprect.w = 32;
	shiprect.h = 32;

	SDL_Rect bulletrect;
	bulletrect.w = 16;
	bulletrect.h = 16;

	SDL_Rect indicrect;
	indicrect.x = 16;
	indicrect.y = 568;
	indicrect.w = 0;
	indicrect.h = 16;

	SDL_Rect pauserect;
	pauserect.x = 258;
	pauserect.y = 280;
	pauserect.w = 284;
	pauserect.h = 40;

	SDL_Rect erect;

	/* other stuff */
	const Uint32 color_blue = SDL_MapRGB(env->screen->format, 0, 0, 255);

	for(i = 0;i<SCREENE;i++)
		elist[i].type = 0;
	for(i = 0;i<SCREENBUL;i++)
		bullets[i].type = 0;
	for(i = 0;i<SCREENEX;i++)
		explo[i].type = 0;


	/* ************************************************************** */
	// GAMEPLAY VARIABLES
	int stripindex = 0;

	float strippos = -168;
	float bgpos = -600;
	float overlaypos = -600;
	long tickslastupdate = 0;
	int bossloop = 0;

	float invuln = 0;
	float bosstimer = 0;

	int spawnpos = 0;
	int glevel = 1;
	int lives;

	float shipx;
	float shipy;
	int shipimg;
	float cooldown = 0.0f;
	float urate, oldrate = 1;
	float rate = 1;
	float speedbar = 0;
	float qboss;
	int gpause = 0;

//	int rate = 2;
	unsigned char keysdown[7] = {0};
	unsigned char joystate[7] = {0};

	glevel = level;
	gpause = 0;
	bosstimer = 0;
	bossloop = 0;

	strippos = -168;
	stripindex = 0;
	bgpos = -600;
	invuln = 0;
	spawnpos = 0;
	lives = 5;
	shipx = 400;
	shipy = 500;
	shipimg = 1;
	cooldown = 0.0f;
	rate = 1;
	speedbar = 0;

	for (i = 0;i<7;i++)
		keysdown[i] = 0;

	tickslastupdate = SDL_GetTicks();

	int state = STATE_GAME;
	do
	{
		/* handle input */
		SDL_Event event;

		/* Check for events */
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_ACTIVEEVENT:
					if ( ( ( event.active.state & SDL_APPACTIVE ) || ( event.active.state & SDL_APPINPUTFOCUS ) ) &&
						event.active.gain == 0) {
						gpause = 1;
					}
					break;
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == env->KEY_UP)
						keysdown[0] = 1;
					else if (event.key.keysym.sym == env->KEY_DOWN)
						keysdown[1] = 1;
					else if (event.key.keysym.sym == env->KEY_LEFT)
						keysdown[2] = 1;
					else if (event.key.keysym.sym == env->KEY_RIGHT)
						keysdown[3] = 1;
					else if (event.key.keysym.sym == env->KEY_FIRE)
						keysdown[4] = 1;
					else if (event.key.keysym.sym == env->KEY_SPEED_DOWN)
						keysdown[5] = 1;
					else if (event.key.keysym.sym == env->KEY_SPEED_UP)
						keysdown[6] = 1;
					break;

				case SDL_KEYUP:
					if (event.key.keysym.sym == env->KEY_UP)
						keysdown[0] = 0;
					else if (event.key.keysym.sym == env->KEY_DOWN)
						keysdown[1] = 0;
					else if (event.key.keysym.sym == env->KEY_LEFT)
						keysdown[2] = 0;
					else if (event.key.keysym.sym == env->KEY_RIGHT)
						keysdown[3] = 0;
					else if (event.key.keysym.sym == env->KEY_FIRE)
						keysdown[4] = 0;
					else if (event.key.keysym.sym == env->KEY_SPEED_DOWN)
						keysdown[5] = 0;
					else if (event.key.keysym.sym == env->KEY_SPEED_UP)
						keysdown[6] = 0;

					else if (event.key.keysym.sym == env->KEY_QUIT)
						state = STATE_UI;

					else if (event.key.keysym.sym == env->KEY_PAUSE)
					{
						if (gpause == 0) {
							if (env->mus_on == 1) {
								Mix_HaltMusic();
								Mix_PlayMusic(music_pause, -1);
							}
							gpause = 1;
						} else {
							if (env->mus_on == 1) {
								Mix_HaltMusic();
								Mix_PlayMusic(music, -1);
							}
							gpause = 0;
						}
					}
					break;
				case SDL_QUIT:
					state = STATE_QUIT;
			}
		}

			/* here we use the joystick to fake keypresses : ) */
/*			if (joysticks>0)
			{
				axis = SDL_JoystickGetAxis(joy, 0);
				if (axis < -10923) {
					joystate[2] = 1;
					joystate[3] = 0;
				} else if (axis > 10923) {
					joystate[2] = 0;
					joystate[3] = 1;
				} else {
					joystate[2] = 0;
					joystate[3] = 0;
				}

				axis = SDL_JoystickGetAxis(joy, 1);
				if (axis < -10923) {
					joystate[0] = 0;
					joystate[1] = 1;
				} else if (axis > 10923) {
					joystate[0] = 1;
					joystate[1] = 0;
				} else {
					joystate[0] = 0;
					joystate[1] = 0;
				}

				button = SDL_JoystickGetButton(joy, 0);
				if (button) joystate[6] = 1; else joystate[6] = 0;
				button = SDL_JoystickGetButton(joy, 1);
				if (button) joystate[4] = 1; else joystate[4] = 0;
				button = SDL_JoystickGetButton(joy, 2);
				if (button) joystate[5] = 1; else joystate[5] = 0;
			}
		}*/

		/* update */
		{
			int i, j;
			float tempticks;

			tempticks = (float)SDL_GetTicks();

			urate = rate * (tempticks-(float)tickslastupdate)/16.666667f;
			tickslastupdate = (long)tempticks;

			if (!gpause) {

				if (cooldown>0.0f) cooldown -= urate;
				if (keysdown[0] != 0 || joystate[0] != 0)
					shipy = shipy-(urate*SCROLLSPEED / rate);
				else if (keysdown[1] != 0 || joystate[1] != 0)
					shipy = shipy+(urate * SCROLLSPEED / rate);
				if (keysdown[2] != 0 || joystate[2] != 0)
				{
					shipx = shipx-(urate * SCROLLSPEED / rate);
					shipimg = 0; }
				else if (keysdown[3] != 0 || joystate[3] != 0)
				{
					shipx = shipx+(urate * SCROLLSPEED / rate);
					shipimg = 2;
				} else shipimg = 1;

				if (( joystate[4] != 0 || keysdown[4] != 0) && cooldown <= 0.0f)
				{
					createbullet(shipx-32, shipy-16, 1, 90, 7.5);
					createbullet(shipx+32, shipy-16, 1, 90, 7.5);
					cooldown = 8;
					playnoise(P_SHOT, rate, shipx);
				}

				if ((keysdown[5] != 0 || joystate[5] != 0) && speedbar>0)
				{
					rate = 0.5;
				}else if (keysdown[6] != 0 || joystate[6] != 0 )
				{
					rate = 2;
				}else {
					rate = 1;
				}

				if (rate<oldrate) playnoise(S_DOWN, 1, 400);
				if (rate>oldrate) playnoise(S_UP, 1, 400);
				oldrate = rate;

				if (shipx<0) shipx = 0;
				if (shipx>800) shipx = 800;
				if (shipy<0) shipy = 0;
				if (shipy>600) shipy = 600;

				if (rate == 2) speedbar += urate*4;
				if (rate == 0.5) speedbar -= urate*16;

				if (speedbar>768) speedbar = 768;
				if (speedbar<0) speedbar = 0;

				for (i = 0;i<SCREENEX;i++)
				{
					if (explo[i].type != 0)
					{
						explo[i].img += urate;
						explo[i].y += (urate * SCROLLSPEED);
						if (explo[i].img >= 5*EXPLOSPEED) {
							if (explo[i].type == 3) {
								state = STATE_VICTORY;
							} else if (explo[i].type == 1 && lives <= 0) { state = STATE_GAMEOVER;
							} else {explo[i].type = 0;}
						}
					}
				}

				for (i = 0;i<SCREENBUL;i++)
				{
					if (bullets[i].type != 0)
					{
						bullets[i].x = bullets[i].x+(urate * (bullets[i].speed*sintbl[(bullets[i].theta+90) % 360]));
						bullets[i].y = bullets[i].y-(urate * (bullets[i].speed*sintbl[bullets[i].theta]));
						if (bullets[i].type == 1)
						{
							for (j = 0;j<SCREENE; j++)
							{
								if (elist[j].type != 0)
								{
									if ((elist[j].x-(img_enemy[elist[j].type-1]->w/2)<bullets[i].x) && (elist[j].x+(img_enemy[elist[j].type-1]->w/2)>bullets[i].x) && (elist[j].y-(img_enemy[elist[j].type-1]->h/2)<bullets[i].y) && (elist[j].y+(img_enemy[elist[j].type-1]->h/2)>bullets[i].y))
									{
										bullets[i].type = 0;
										elist[j].life--;
									}
								}
							}
						}else if (invuln <= 0) {
							if ((shipx-32<bullets[i].x) && (shipx+32>bullets[i].x) && (shipy-32<bullets[i].y) && (shipy+32>bullets[i].y))
							{
								bullets[i].type = 0;
								invuln = 100;
								createexplosion(shipx, shipy, 1);
								playnoise(P_DIE, rate, shipx);
								lives--;
							}
						}
						if (bullets[i].y<0 || bullets[i].y>600 || bullets[i].x<0 || bullets[i].x>800) bullets[i].type = 0;
					}
				}

				for (i = 0;i<SCREENE;i++)
				{
					if (elist[i].type != 0)
					{
						switch (elist[i].type) {
							case 1:                      // stationary turret
								elist[i].arg1 += urate;
								if (elist[i].arg1>10)
								{
									createbullet(elist[i].x, elist[i].y+16, 2, 270, 5.0);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								elist[i].y += SCROLLSPEED*urate;
								break;
							case 2:                     // shrapnel mine
								elist[i].arg1 += urate;
								if (elist[i].arg1>5)
								{
									createbullet(elist[i].x, elist[i].y, 3, elist[i].arg2, 5.0);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg2 += 15;
									if (elist[i].arg2>359) elist[i].arg2 = 0;
									elist[i].arg1 = 0;
								}
								elist[i].y += SCROLLSPEED*urate;
								break;
							case 3:                      // single homer
								elist[i].x += SCROLLSPEED*2*urate*elist[i].arg2;
								if (elist[i].arg3 == 0 && ((elist[i].x>elist[i].arg1 && elist[i].arg2>0) || (elist[i].x<elist[i].arg1 && elist[i].arg2<0)))
								{
									createenemy(elist[i].x, elist[i].y, 246, 0, 270, 1);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg3 = 1;
								}
								elist[i].y += SCROLLSPEED*urate;
								break;
							case 4:                          // charger
								elist[i].arg1 += urate;
								if (elist[i].arg1>5)
								{
									createbullet(elist[i].x, elist[i].y+16, 2, 265+(rand()%10), 10.0);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								elist[i].y += SCROLLSPEED*1.5*urate;
								break;
							case 5:                          // depth charge
								elist[i].arg1 += urate;
								if (elist[i].arg1>elist[i].arg2)
								{
									createbullet(elist[i].x, elist[i].y+16, 5, 0, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 45, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 90, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 135, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 180, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 225, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 270, 10.0);
									createbullet(elist[i].x, elist[i].y+16, 5, 315, 10.0);
									playnoise(E_SHOT, rate, elist[i].x);
									createexplosion(elist[i].x, elist[i].y, 4);
									elist[i].type = 0;
									elist[i].arg1 = 0;
								}
								elist[i].y += SCROLLSPEED*urate;
								break;
							case 6:                          //side rocket guy
								elist[i].y -= urate;
								elist[i].arg1 += urate;
								if (elist[i].arg1>1)
								{
									if (elist[i].arg3 == 1)
									{
										elist[i].arg2 -= 1;
										if (elist[i].arg2 <= 0) { elist[i].arg3 = 0; elist[i].arg2 = 0;}
									} else {
										createbullet(elist[i].x, elist[i].y-32, 4, 0, 10.0);
										createbullet(elist[i].x, elist[i].y-4, 4, 0, 10.0);
										createbullet(elist[i].x, elist[i].y+22, 4, 0, 10.0);
										createbullet(elist[i].x, elist[i].y-32, 4, 180, 10.0);
										createbullet(elist[i].x, elist[i].y-4, 4, 180, 10.0);
										createbullet(elist[i].x, elist[i].y+22, 4, 180, 10.0);
										playnoise(E_SHOT, rate, elist[i].x);
										elist[i].arg2 += 1;
										if (elist[i].arg2 >= 20) elist[i].arg3 = 1; }
									elist[i].arg1 = 0;
								}
								break;
							case 7:                          //brick!
								elist[i].y += SCROLLSPEED*urate;
								break;
							case 8:                          //carrier: launches fighters
								elist[i].arg1 += urate;
								if (elist[i].arg1>100)
								{
									createenemy(elist[i].x, elist[i].y, 9, 0, 0, 2);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								break;
							case 9:                         //carrier fighter!
								elist[i].y += SCROLLSPEED*1.5*urate;
								if (elist[i].x<shipx) elist[i].x += urate;
								if (elist[i].x>shipx) elist[i].x -= urate;
								elist[i].arg1 += urate;
								if (elist[i].arg1>15)
								{
									createbullet(elist[i].x-8, elist[i].y, 5, 270, 10.0);
									createbullet(elist[i].x+8, elist[i].y, 5, 270, 10.0);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								break;
							case 10:                         //uberbomb.
								elist[i].y += urate*SCROLLSPEED;
								if (elist[i].y>300)
								{
									elist[i].type = 0;
									createexplosion(400, 300, 5);
									speedbar = 0;
								}
								break;
							case 242:
							case 243:
							case 244:             // space station!
								elist[i].arg1 += urate;
								if (elist[i].arg1>7)      //animate
								{
									elist[i].type += 1; if (elist[i].type == 245) elist[i].type = 242;
									elist[i].arg1 = 0;
								}
								if (elist[i].y<128) elist[i].y += urate*SCROLLSPEED; else {
									// do all battle actions here.
									elist[i].y = 128;
									if (elist[i].arg3>0)   // taking a shot
									{
										if (elist[i].arg3 == 1)  // machine gun!
										{
											elist[i].arg3 = 101;
										}
										if (elist[i].arg3 == 3)
										{
											bosstimer += urate;
											if (bosstimer>5)
											{
												for (qboss = 210;qboss <= 330;qboss += 30)
													createenemy(elist[i].x, elist[i].y, 246, 0, qboss, 1);
												playnoise(E_SHOT, rate, elist[i].x);
												bosstimer = 0;
												elist[i].arg2 += 1; if (elist[i].arg2>4) {elist[i].arg2 = 0; elist[i].arg3 = 0;}
											}
										}
										if (elist[i].arg3 == 2)
										{
											createenemy(elist[i].x, elist[i].y, 10, 0, 0, 1);
											playnoise(E_SHOT, rate, elist[i].x);
											elist[i].arg3 = 0;
											bosstimer = 0;
											elist[i].arg2 = 0;
										}
										if (elist[i].arg3 == 5)
										{
											createenemy(elist[i].x-50, elist[i].y, 2, 0, 0, 5);
											createenemy(elist[i].x+50, elist[i].y, 2, 0, 180, 5);
											createenemy(elist[i].x, elist[i].y, 5, 0, 20, 5);
											playnoise(E_SHOT, rate, elist[i].x);
											elist[i].arg3 = 0;
											bosstimer = 0;
											elist[i].arg2 = 0;
										}
										if (elist[i].arg3 == 4)
										{
											createenemy(0, 0, 7, 0, 0, 100);
											createenemy(100, 0, 7, 0, 0, 100);
											createenemy(200, 0, 7, 0, 0, 100);
											createenemy(600, 0, 7, 0, 0, 100);
											createenemy(700, 0, 7, 0, 0, 100);
											createenemy(800, 0, 7, 0, 0, 100);
											playnoise(E_SHOT, rate, elist[i].x);
											elist[i].arg3 = 0;
											bosstimer = 0;
											elist[i].arg2 = 0;
										}
										if (elist[i].arg3>100)
										{
											bosstimer += urate;
											if (bosstimer>1)
											{
												bosstimer = 0;
												elist[i].arg3 += 1;
												elist[i].arg2 = 180*atan2((-shipy+elist[i].y), (shipx-elist[i].x))/PI;
												if (elist[i].arg2<0) elist[i].arg2 += 360;
												createbullet(elist[i].x, elist[i].y, 5, elist[i].arg2, 10);
												playnoise(E_SHOT, rate, elist[i].x);
												elist[i].arg2 = 0;
												if (elist[i].arg3>200) {elist[i].arg3 = 0; }
											}
										}
									} else {
										elist[i].arg2 += urate;
										if (elist[i].arg2>150) { elist[i].arg2 = 0; elist[i].arg3 = bossloop; bossloop += 1; if (bossloop>5) bossloop = 1; }
									}
								}

								break;
							case 245:
							case 246:
							case 247:              // homing rocket
								if (elist[i].arg2<225) elist[i].type = 245;
								else if (elist[i].arg2>315) elist[i].type = 247;
								else elist[i].type = 246;

								elist[i].arg1 += urate;
								if (elist[i].arg1 > 20)     // can't adjust angle at first
								{
									elist[i].arg3 = (180*atan2((-shipy+elist[i].y), (shipx-elist[i].x))/PI);
									if (elist[i].arg3<0) elist[i].arg3 += 360;
									elist[i].arg2 += (elist[i].arg2<elist[i].arg3?2*urate:2*(-urate));
									if (elist[i].arg2>330) elist[i].arg2 = 330;
									if (elist[i].arg2<210) elist[i].arg2 = 210;
								}
								elist[i].x += 10*urate*sintbl[(int)(elist[i].arg2+90) % 360];
								elist[i].y -= 10*urate*sintbl[(int)elist[i].arg2];
								break;
							case 248:                // laser boss helper
								elist[i].arg1 += urate;
								if (elist[i].arg1>20)
									elist[i].type = 0;
								else
								{
									elist[i].arg3 += urate;
									if (elist[i].arg3>1)
									{
										elist[i].arg3 = 0;
										createbullet(elist[i].x, elist[i].y, 5, elist[i].arg2, 10);
										playnoise(E_SHOT, rate, elist[i].x);
									}
								}
								break;
							case 249:                // shotgun boss
								elist[i].arg1 += urate;
								if (elist[i].arg1>100)
								{
									playnoise(E_SHOT, rate, elist[i].x);
									if (elist[i].arg3 == 0) elist[i].arg1 = 240;
									if (elist[i].arg3 == 1 || elist[i].arg3 == 7) elist[i].arg1 = 255;
									if (elist[i].arg3 == 2 || elist[i].arg3 == 6) elist[i].arg1 = 270;
									if (elist[i].arg3 == 3 || elist[i].arg3 == 5) elist[i].arg1 = 285;
									if (elist[i].arg3 == 4) elist[i].arg1 = 300;
									elist[i].arg3 = elist[i].arg3+1;
									if (elist[i].arg3 == 8) elist[i].arg3 = 0;
									for (elist[i].arg2 = 210;elist[i].arg2<elist[i].arg1-5;elist[i].arg2++)
										createbullet(elist[i].x, elist[i].y, 2, elist[i].arg2, 7.5);
									for (elist[i].arg2 = elist[i].arg1+5;elist[i].arg2 <= 330;elist[i].arg2++)
										createbullet(elist[i].x, elist[i].y, 2, elist[i].arg2, 7.5);
									elist[i].arg1 = 0;
								}
								break;
							case 250:               //shrapnel boss
								elist[i].arg1 += urate;
								elist[i].x += elist[i].arg2;
								if (elist[i].x<128) elist[i].arg2 = 1;
								if (elist[i].x>672) elist[i].arg2 = -1;
								if (elist[i].arg1>150)
								{
									createenemy(elist[i].x, elist[i].y, 2, 0, 0, 5);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								break;
							case 251:               //laser boss
								elist[i].arg1 += urate;
								if (elist[i].arg1>20)
								{
									elist[i].arg2 = 180*atan2((-shipy+elist[i].y), (shipx-elist[i].x))/PI;
									if (elist[i].arg2<0) elist[i].arg2 += 360;
									createenemy(elist[i].x, elist[i].y, 248, 0, elist[i].arg2, 1);
									elist[i].arg1 = 0;
								}
								break;
							case 252:               //rocket boss
								elist[i].arg1 += urate;
								elist[i].x += elist[i].arg2;
								if (elist[i].x<300) elist[i].arg2 = 1;
								if (elist[i].x>500) elist[i].arg2 = -1;
								if (elist[i].arg1>50)
								{
									for (elist[i].arg3 = 210;elist[i].arg3 <= 330;elist[i].arg3 += 30)
										createenemy(elist[i].x, elist[i].y, 246, 0, elist[i].arg3, 1);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								break;
							case 253:               //dcboss
								elist[i].arg1 += urate;
								elist[i].x += elist[i].arg2;
								if (elist[i].x<128) elist[i].arg2 = 1;
								if (elist[i].x>672) elist[i].arg2 = -1;
								if (elist[i].arg1>100)
								{
									createenemy(elist[i].x, elist[i].y, 5, 0, (shipy-elist[i].y)/SCROLLSPEED, 5);
									playnoise(E_SHOT, rate, elist[i].x);
									elist[i].arg1 = 0;
								}
								break;
							default:
								break;
						}
						if (elist[i].type != 0){
							if (invuln == 0)
								if ((elist[i].x-(img_enemy[elist[i].type-1]->w/2)<shipx+32) && (elist[i].x+(img_enemy[elist[i].type-1]->w/2)>shipx-32) && (elist[i].y-(img_enemy[elist[i].type-1]->h/2)<shipy+32) && (elist[i].y+(img_enemy[elist[i].type-1]->h/2)>shipy-32) )
								{
									elist[i].life--;
									lives--;
									invuln = 100;
									createexplosion(shipx, shipy, 1);
								}
							if (elist[i].life <= 0) {*score += scores[elist[i].type]; createexplosion(elist[i].x, elist[i].y, 2);
								if (elist[i].type>248 || (elist[i].type > 241 && elist[i].type < 245)) {
									// Just for fun, make one of these big blue bombs go off.
									if (elist[i].type > 241 && elist[i].type < 245)
										createexplosion(elist[i].x, elist[i].y, 5);
									createexplosion(elist[i].x-32, elist[i].y-32, 1);
									createexplosion(elist[i].x+32, elist[i].y-32, 3);
									createexplosion(elist[i].x-32, elist[i].y+32, 3);
									createexplosion(elist[i].x+32, elist[i].y+32, 1);
								}
								elist[i].type = 0;}
							if (elist[i].y<0 || elist[i].y>600 || elist[i].x<0 || elist[i].x>800) elist[i].type = 0;
						}
					}
				}

				if (invuln>0) {invuln -= urate; shipimg = 3;}
				if (invuln<0) invuln = 0;

			}
		}

		/* draw */
		if (gpause == 1)
		{
			SDL_BlitSurface(img_pause, NULL, env->screen, &pauserect);
		} else {
			bgpos += .5 * urate * SCROLLSPEED;
			strippos += urate * SCROLLSPEED;
			overlaypos += 2*urate * SCROLLSPEED;
			if (overlaypos >= 0) overlaypos = -600;
			if (strippos >= -40) {
				strippos = -168;
				
				stripindex++;
				
				while (slist[spawnpos].when <= stripindex)
				{
					createenemy(slist[spawnpos].x, slist[spawnpos].y, slist[spawnpos].type, slist[spawnpos].arg1, slist[spawnpos].arg2, slist[spawnpos].life);

					if (slist[spawnpos].type>248)
					{
						Mix_FreeMusic(music);
						glevel = 0;
						if (music_boss != NULL) Mix_PlayMusic(music_boss, -1);
					}
					spawnpos++;
				}
			}
			if (bgpos >= 0) bgpos = -600;
			bgrect.y = (int)bgpos;
			SDL_BlitSurface(bg, NULL, env->screen, &bgrect);
			bgrect.y = (int)bgpos+600;
			SDL_BlitSurface(bg, NULL, env->screen, &bgrect);

			if (stripindex>121) stripindex = 121;

			for(i = 0;i<6;i++)
			{
				striprect.y = (int)strippos+128*i;
				SDL_BlitSurface(strips[stripslist[stripindex+6-i]], NULL, env->screen, &striprect);
			}

			for (i = 0;i<SCREENBUL;i++)
			{
				if (bullets[i].type != 0)
				{
					bulletrect.x = (int)(bullets[i].x+.5)-4;
					bulletrect.y = (int)(bullets[i].y+.5)-4;
					SDL_BlitSurface(img_shot[bullets[i].type-1], NULL, env->screen, &bulletrect);
				}
			}

			for (i = 0;i<SCREENE;i++)
			{
				if (elist[i].type != 0)
				{
					erect.w = img_enemy[elist[i].type-1]->w;
					erect.h = img_enemy[elist[i].type-1]->h;
					erect.x = (int)(elist[i].x+.5)-(erect.w/2);
					erect.y = (int)(elist[i].y+.5)-(erect.h/2);
					SDL_BlitSurface(img_enemy[elist[i].type-1], NULL, env->screen, &erect);
				}
			}

			shiprect.x = (int)shipx-32;
			shiprect.y = (int)shipy-32;
			SDL_BlitSurface(img_player[shipimg], NULL, env->screen, &shiprect);

			for (i = 0;i<SCREENEX;i++)
			{
				if (explo[i].type != 0)
				{
					erect.w = img_explo[5*(explo[i].type-1)]->w;
					erect.h = img_explo[5*(explo[i].type-1)]->h;
					erect.x = (int)(explo[i].x+.5)-(erect.w/2);
					erect.y = (int)(explo[i].y+.5)-(erect.h/2);
					SDL_BlitSurface(img_explo[(explo[i].type-1)*5+((int)explo[i].img)/EXPLOSPEED], NULL, env->screen, &erect);
				}
			}

			/* Speed lines */
			bgrect.y = (int)overlaypos;
			SDL_BlitSurface(img_overlay[(int)rate], NULL, env->screen, &bgrect);
			bgrect.y = (int)overlaypos+600;
			SDL_BlitSurface(img_overlay[(int)rate], NULL, env->screen, &bgrect);
		}

		/* HUD items */
		// lives
		SDL_BlitSurface(img_player[1], NULL, env->screen, NULL);
		blit_number(env->screen, 80, 16, lives - 1);
		// score
		blit_number(env->screen, 768, 16, *score);
		// time meter
		indicrect.w = (int)speedbar;
		SDL_FillRect(env->screen, &indicrect, color_blue);

		SDL_Flip (env->screen);

		SDL_Delay (1);
	} while (state == STATE_GAME);

	/* *********************************************************************** */
	/* shut down */
	if (glevel<6) {
		for (i = 0;i<42;i++)
			if (strips[i] != NULL)
				SDL_FreeSurface(strips[i]); }

	Mix_HaltChannel(-1);

	Mix_FreeMusic(music);

	return state;
}

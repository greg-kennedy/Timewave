/* game.c - gameplay */

/*
  All gameplay takes place on an 800x600 screen.
*/
#include "game.h"

#include <limits.h>

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef min
#define min(a,b)    (((a) < (b)) ? (a) : (b))
#endif

#include "common.h"

#include "meta_play.h"

#define SCROLLSPEED 1.0/15.0
#define EXPLOSPEED 0.005
#define SHIPSPEED 0.25
#define BARSPEED 0.25

/* num imgs to load */
#define MAXSHIPS 4
#define MAXBULS 5
#define MAXENS 24
#define MAXEXP 5
#define MAXSTRIPS 42

/* max num on screen */
#define SCREENBUL 500
#define SCREENE 32
#define ALLSPAWN 128
#define SCREENEX 16

#define LVLSIZE 128

/* Sample names */
#define SLOW 0
#define NORMAL 1
#define FAST 2

#define P_DIE 0
#define E_DIE 1
#define B_DIE 2
#define P_SHOT 3
#define E_SHOT 4
#define E_LAUNCH 5

#define S_UP 6
#define S_DOWN 7

/* ************************************************************** */
// STRUCT DEFINITIONS
//  loaded from file
struct spawninfo {
	unsigned char when;
	unsigned char type;
	unsigned short x;
	unsigned short y;
	short arg1;
	short arg2;
};

// object pools
struct bullet {
	unsigned char type;
	float x;
	float y;
	float dx;
	float dy;
};

struct enemy {
	unsigned char type;
	short life;
	float x;
	float y;
	short arg1;
	short arg2;

	short timer;
};

struct explosion {
	unsigned char type;
	short x;
	float y;
	float timer;
};

/* ************************************************************** */
// CONSTANTS
static const unsigned int scores[MAXENS] = {
	50, 250, 100, 200, 250, 250, 250, 500, 0,
	500, 25, 0,
	10, 10, 10, 0,
	100000, 100000, 100000,
	10000, 20000, 30000, 40000, 50000
};

static const short hp[MAXENS] = {
	1, 4, 2, 3, 4, 4, 4, 7, SHRT_MAX,
	7, 1, SHRT_MAX,
	1, 1, 1, 1,
	500, 500, 500, 100, 100, 100, 100, 100
};

/* ************************************************************** */
// OBJECT POOLS
static struct bullet bullets[SCREENBUL];
static struct enemy elist[SCREENE];
static struct explosion explo[SCREENEX];

/* ************************************************************** */
// PRELOADED Resources
static Mix_Chunk * chunk_up;
static Mix_Chunk * chunk_down;
static Mix_Chunk * chunk_sfx[3][6];

static Mix_Music * music_boss;

static SDL_Surface * bg;
static SDL_Surface * img_pause;
static SDL_Surface * img_overlay[3];
static SDL_Surface * img_player[MAXSHIPS];
static SDL_Surface * img_shot[MAXBULS];
static SDL_Surface * img_enemy[MAXENS];
static SDL_Surface * img_explo[MAXEXP * 5];

/* ************************************************************** */
// SHARED GAME DATA INIT / PRELOAD
int init_game(struct env_t * env)
{
	int i;
	char buffer[30];
	/* LOAD ALL IMAGES */
	bg = load_image("img/gfx/bg.png", 0);
	img_pause = load_image("img/ui/paused.png", 3);

	for (i = 0; i < 3; i++) {
		sprintf(buffer, "img/gfx/top%d.png", i);
		img_overlay[i] = load_image(buffer, 3);
	}

	for (i = 0; i < MAXSHIPS; i++) {
		sprintf(buffer, "img/gfx/ship%d.png", i);
		img_player[i] = load_image(buffer, 1);
	}

	for (i = 0; i < MAXENS; i++) {
		sprintf(buffer, "img/gfx/en%d.png", i);
		img_enemy[i] = load_image(buffer, 1);
	}

	for (i = 0; i < MAXEXP * 5; i++) {
		sprintf(buffer, "img/gfx/exp%d.png", i);
		img_explo[i] = load_image(buffer, 3);
	}

	for (i = 0; i < MAXBULS; i++) {
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
		chunk_sfx[SLOW][E_LAUNCH] = load_sample("audio/slaunch.wav");
		chunk_sfx[NORMAL][P_DIE] = load_sample("audio/ex2.wav");
		chunk_sfx[NORMAL][E_DIE] = load_sample("audio/ex1.wav");
		chunk_sfx[NORMAL][B_DIE] = load_sample("audio/ex3.wav");
		chunk_sfx[NORMAL][P_SHOT] = load_sample("audio/bul2.wav");
		chunk_sfx[NORMAL][E_SHOT] = load_sample("audio/bul1.wav");
		chunk_sfx[NORMAL][E_LAUNCH] = load_sample("audio/launch.wav");
		chunk_sfx[FAST][P_DIE] = load_sample("audio/fex2.wav");
		chunk_sfx[FAST][E_DIE] = load_sample("audio/fex1.wav");
		chunk_sfx[FAST][B_DIE] = load_sample("audio/fex3.wav");
		chunk_sfx[FAST][P_SHOT] = load_sample("audio/fbul2.wav");
		chunk_sfx[FAST][E_SHOT] = load_sample("audio/fbul1.wav");
		chunk_sfx[FAST][E_LAUNCH] = load_sample("audio/flaunch.wav");
	}

	// BOSS music
	if (env->mus_works && env->mus_on)
		music_boss = load_music("audio/boss.mod", 0);

	return 0;
}

/* ************************************************************** */
// SHARED GAME FREE
void free_game(void)
{
	int i, j;
	/* Images */
	SDL_FreeSurface(bg);
	SDL_FreeSurface(img_pause);

	for (i = 0; i < MAXSHIPS; i++)
		SDL_FreeSurface(img_player[i]);

	for (i = 0; i < MAXEXP * 5; i++)
		SDL_FreeSurface(img_explo[i]);

	for (i = 0; i < 3; i++)
		SDL_FreeSurface(img_overlay[i]);

	for (i = 0; i < MAXBULS; i++)
		SDL_FreeSurface(img_shot[i]);

	for (i = 0; i < MAXENS; i++)
		SDL_FreeSurface(img_enemy[i]);

	// Sound FX
	Mix_FreeChunk(chunk_up);
	Mix_FreeChunk(chunk_down);

	for (i = 0; i < 3; i ++)
		for (j = 0; j < 6; j ++)
			Mix_FreeChunk(chunk_sfx[i][j]);

	// Music
	Mix_FreeMusic(music_boss);
}

// Trigger a sound effect, with panning.
static void playnoise(const int sample, const int rate, const float x)
{
	Mix_Chunk * chunk;

	if (sample == S_UP) chunk = chunk_up;
	else if (sample == S_DOWN) chunk = chunk_down;
	else chunk = chunk_sfx[rate][sample];

	if (chunk != NULL) {
		int channel;

		if (sample == S_UP || sample == S_DOWN) {
			Mix_HaltChannel(0);
			channel = Mix_PlayChannel(0, chunk, 0);
		} else if (sample == P_SHOT) {
			Mix_HaltChannel(1);
			channel = Mix_PlayChannel(1, chunk, 0);
		} else if (sample == P_DIE) {
			Mix_HaltChannel(2);
			channel = Mix_PlayChannel(2, chunk, 0);
		} else if (sample == E_DIE || sample == B_DIE) {
			Mix_HaltChannel(3);
			channel = Mix_PlayChannel(3, chunk, 0);
		} else {
			// Find the oldest
			channel = Mix_GroupOldest(0);

			if (channel == -1) {
				// no channel playing or allocated...
				// perhaps just search for an available channel...
				channel = Mix_PlayChannel(-1, chunk, 0);
			} else {
				Mix_HaltChannel(channel);
				channel = Mix_PlayChannel(channel, chunk, 0);
			}
		}

		if (channel >= 0) {
			int p = x * 0.3199999630451202392578125f;
			Mix_SetPanning(channel, 255 - p, p);
		}
	}
}

// Add a bullet to the bullet object pool
static int createbullet(const unsigned char type, const unsigned char theta, const float x, const float y, const float speed)
{
	int i;

	for (i = 0; i < SCREENBUL; i++)
		if (bullets[i].type == 0) {
			bullets[i].type = type;
			bullets[i].x = x - img_shot[type - 1]->w / 2;
			bullets[i].y = y - img_shot[type - 1]->h / 2;
			bullets[i].dx = speed * cos(M_PI / 128.0 * theta);
			bullets[i].dy = speed * -sin(M_PI / 128.0 * theta);
			break;
		}

	return (i == SCREENBUL);
}

// Create an explosion in the explosion pool centered at x, y
static int createexplosion(const unsigned char type, const short x, const short y)
{
	int i;

	for (i = 0; i < SCREENEX; i++)
		if (explo[i].type == 0) {
			explo[i].type = type;
			explo[i].timer = 0;
			explo[i].x = x - img_explo[5 * (type - 1)]->w / 2;
			explo[i].y = y - img_explo[5 * (type - 1)]->h / 2;
			break;
		}

	return (i == SCREENEX);
}

// Create an enemy in the enemy pool
static int createenemy(const unsigned char type, const short x, const short y, const short arg1, const short arg2)
{
	int i;

	for (i = 0; i < SCREENE; i++)
		if (elist[i].type == 0) {
			elist[i].type = type;
			elist[i].life = hp[type - 1];
			elist[i].x = x;
			elist[i].y = y;
			elist[i].arg1 = arg1;
			elist[i].arg2 = arg2;
			elist[i].timer = 0;
			break;
		}

	return (i == SCREENE);
}

static Uint32 get_pixel(const SDL_Surface * surface, const int x, const int y)
{
	const int bpp = surface->format->BytesPerPixel;
	/* Here p is the address to the pixel we want to retrieve */
	const Uint8 * p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch (bpp) {
	case 1:
		return *p;

	case 2:
		return *(Uint16 *)p;

	case 3:
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		return p[0] << 16 | p[1] << 8 | p[2];
#else
		return p[0] | p[1] << 8 | p[2] << 16;
#endif

	case 4:
		return *(Uint32 *)p;

	default:
		return 0;       /* shouldn't happen, but avoids warnings */
	}
}

// Pixel-perfect collision between two transparent surfaces
static int check_pixel_collision(const int x1, const int y1, const int x2, const int y2, SDL_Surface * img1, SDL_Surface * img2)
{
	int offset_1_x, offset_2_x, offset_1_y, offset_2_y, w, h, x, y;

	// early rejection
	if (x1 + img1->w <= x2) return 0;

	if (x2 + img2->w <= x1) return 0;

	if (y1 + img1->h <= y2) return 0;

	if (y2 + img2->h <= y1) return 0;

	// determine the overlapping rectangle boundaries
	if (x1 < x2) {
		offset_1_x = x2 - x1;
		offset_2_x = 0;
	} else {
		offset_1_x = 0;
		offset_2_x = x1 - x2;
	}

	w = min(img1->w - offset_1_x, img2->w - offset_2_x);

	if (y1 < y2) {
		offset_1_y = y2 - y1;
		offset_2_y = 0;
	} else {
		offset_1_y = 0;
		offset_2_y = y1 - y2;
	}

	h = min(img1->h - offset_1_y, img2->h - offset_2_y);

	/*
	fprintf(stderr, "Checking collision between img1 (%d, %d, %d, %d) and img2(%d, %d, %d, %d): o1x=%d o1y=%d o2x=%d o2y=%d w=%d h=%d\n",
		x1, y1, img1->w, img1->h,
		x2, y2, img2->w, img2->h,
		offset_1_x, offset_1_y, offset_2_x, offset_2_y, w, h
	);
	*/

	if (SDL_MUSTLOCK(img1)) SDL_LockSurface(img1);

	if (SDL_MUSTLOCK(img2)) SDL_LockSurface(img2);

	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			Uint8 r, g, b;
			SDL_GetRGB(get_pixel(img1, x + offset_1_x, y + offset_1_y), img1->format, &r, &g, &b);

			if (r != 0xFF || g != 0 || b != 0xFF) {
				SDL_GetRGB(get_pixel(img2, x + offset_2_x, y + offset_2_y), img2->format, &r, &g, &b);

				if (r != 0xFF || g != 0 || b != 0xFF) {
					if (SDL_MUSTLOCK(img2)) SDL_UnlockSurface(img2);

					if (SDL_MUSTLOCK(img1)) SDL_UnlockSurface(img1);

					return 1;
				}
			}
		}
	}

	if (SDL_MUSTLOCK(img2)) SDL_UnlockSurface(img2);

	if (SDL_MUSTLOCK(img1)) SDL_UnlockSurface(img1);

	return 0;
}

/* ************************************************************** */
// MAIN GAME LOOP
int state_game(struct env_t * env, Mix_Music * music_pause, const int level, int * score)
{
	int i, j;
	/* ************************************************************** */
	// NON-PRELOADED Resources
	SDL_Surface * strips[MAXSTRIPS] = {NULL};
	/* LOAD DATA FILES FOR LEVEL */
	char buffer[30];
	unsigned char stripslist[LVLSIZE] = { 0 };
	struct spawninfo slist[ALLSPAWN] = { { 0 } };
	sprintf(buffer, "data/%d.lvl", level);
	FILE * fp = fopen(buffer, "r");
	fscanf(fp, "%d\n", &j);

	for (i = 0; i < j; i++) {
		int line = 0;
		fscanf(fp, "%d\n", &line);
		stripslist[i] = line;
	}

	fscanf(fp, "%d\n", &j);

	for (i = 0; i < j; i++) {
		int line[6] = { 0 };
		// this is a workaround for MSVC not supporting %hhu formats
		fscanf(fp, "%d %d %d %d %d %d\n", &line[0], &line[1], &line[2], &line[3], &line[4], &line[5]);
		slist[i].when = line[0];
		slist[i].type = line[1];
		slist[i].x = line[2];
		slist[i].y = line[3];
		slist[i].arg1 = line[4];
		slist[i].arg2 = line[5];
	}

	fclose(fp);
	Mix_Music * music = NULL;

	if (env->mus_works && env->mus_on) {
		sprintf(buffer, "audio/%d.mod", level);
		music = load_music(buffer, 1);
	}

	// load strips - any used in the map (and don't double-load)
	for (i = 0; i < LVLSIZE; i++) {
		if (stripslist[i] && ! strips[stripslist[i]]) {
			sprintf(buffer, "img/strips/%d/%d.png", level, stripslist[i]);
			strips[stripslist[i]] = load_image(buffer, 3);
		}
	}

	/* setup rectangles */
	SDL_Rect striprect = {};
	striprect.w = 800;
	striprect.h = 128;
	SDL_Rect bgrect = {};
	bgrect.w = 800;
	bgrect.h = 600;
	SDL_Rect shiprect = {};
	shiprect.w = 64;
	shiprect.h = 64;
	SDL_Rect bulletrect = {};
	bulletrect.h = 8;
	SDL_Rect indicrect = {};
	indicrect.x = 16;
	indicrect.y = 568;
	indicrect.h = 16;
	SDL_Rect pauserect = {};
	pauserect.x = 258;
	pauserect.y = 280;
	pauserect.w = 284;
	pauserect.h = 40;
	/* other stuff */
	const Uint32 color_blue = SDL_MapRGB(env->screen->format, 0, 0, 255);

	for (i = 0; i < SCREENE; i++)
		elist[i].type = 0;

	for (i = 0; i < SCREENBUL; i++)
		bullets[i].type = 0;

	for (i = 0; i < SCREENEX; i++)
		explo[i].type = 0;

	/* ************************************************************** */
	// GAMEPLAY VARIABLES
	// which strip we are on
	int stripindex = 0;
	// Index into spawn array
	int spawnidx = 0;
	// positions of various rectangles
	float bgpos = 0;
	float strippos = 432;
	float overlaypos = 0;
	// Player positions, lives, info etc
	int lives = 5;
	int shipimg = 1;
	float shipx = 400 - img_player[shipimg]->w / 2;
	float shipy = 500 - img_player[shipimg]->h / 2;
	int cooldown = 0;
	// Time left on player invuln (0 if vuln)
	int invuln = 0;
	// amount of time buildup
	float speedbar = 0;
	// game speed (0=slow 1=normal 2=fast)
	int rate = 1;
	// is game paused?
	int gpause = 0;
	// Final Boss needs extra tick counters for attacks.
	int boss_timer = 0;
	unsigned char keysdown[7] = {0};
	// SDL_Ticks since last update
	unsigned long tickslastupdate = SDL_GetTicks();
	/* Event loop */
	int state = STATE_GAME;

	do {
		/* handle input */
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_ACTIVEEVENT:

				// if the game loses focus, pause it
				if (((event.active.state & SDL_APPACTIVE) || (event.active.state & SDL_APPINPUTFOCUS)) &&
					event.active.gain == 0 && !gpause) {
					if (env->mus_on == 1)
						Mix_PlayMusic(music_pause, -1);

					gpause = 1;
				}

				break;

			// KEYBOARD
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
				else if (event.key.keysym.sym == env->KEY_PAUSE) {
					if (gpause == 0) {
						if (env->mus_on == 1)
							Mix_PlayMusic(music_pause, -1);

						gpause = 1;
					} else {
						if (env->mus_on == 1)
							Mix_PlayMusic(music, -1);

						gpause = 0;
					}
				} else if (env->debug) {
					if (event.key.keysym.sym == SDLK_F2)
						lives ++;
					else if (event.key.keysym.sym == SDLK_F3)
						state = STATE_VICTORY;
				}

				break;

			// JOYSTICK
			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0) { // left-right movement
					if (event.jaxis.value < -4095) {
						keysdown[2] = 1;
						keysdown[3] = 0;
					} else if (event.jaxis.value > 4095) {
						keysdown[2] = 0;
						keysdown[3] = 1;
					} else {
						keysdown[2] = 0;
						keysdown[3] = 0;
					}
				} else if (event.jaxis.axis == 1) { // up-down axis
					if (event.jaxis.value < -4095) {
						keysdown[0] = 1;
						keysdown[1] = 0;
					} else if (event.jaxis.value > 4095) {
						keysdown[0] = 0;
						keysdown[1] = 1;
					} else {
						keysdown[0] = 0;
						keysdown[1] = 0;
					}
				}

				break;

			case SDL_JOYBUTTONDOWN:
				if (event.jbutton.button == env->buttons[0])
					keysdown[4] = 1;
				else if (event.jbutton.button == env->buttons[1])
					keysdown[5] = 1;
				else if (event.jbutton.button == env->buttons[2])
					keysdown[6] = 1;

				break;

			case SDL_JOYBUTTONUP:
				if (event.jbutton.button == env->buttons[0])
					keysdown[4] = 0;
				else if (event.jbutton.button == env->buttons[1])
					keysdown[5] = 0;
				else if (event.jbutton.button == env->buttons[2])
					keysdown[6] = 0;
				else if (event.jbutton.button == env->buttons[3]) {
					if (gpause == 0) {
						if (env->mus_on == 1)
							Mix_PlayMusic(music_pause, -1);

						gpause = 1;
					} else {
						if (env->mus_on == 1)
							Mix_PlayMusic(music, -1);

						gpause = 0;
					}
				}

				break;

			case SDL_QUIT:
				state = STATE_QUIT;
			}
		}

		/* update time step */
		unsigned long tempticks = SDL_GetTicks();
		// calculate the number of ticks between before and now
		unsigned long t_diff = tempticks - tickslastupdate;
		tickslastupdate = tempticks;

		// only do game processing and draw if ticks have passed
		if (t_diff) {
			// game unpaused so make everything happen
			if (!gpause) {
				// lots of movements are driven by scaled rate
				int urate = (1 << rate) * t_diff;

				// ship movement
				if (keysdown[0] != 0) {
					shipy -= (t_diff * SHIPSPEED);

					if (shipy < 0) shipy = 0;
				} else if (keysdown[1] != 0) {
					shipy += (t_diff * SHIPSPEED);

					if (shipy >= 600 - img_player[shipimg]->h) shipy = 599.99993896484375f - img_player[shipimg]->h;
				}

				if (keysdown[2] != 0) {
					shipimg = 0;
					shipx -= (t_diff * SHIPSPEED);

					if (shipx < 0) shipx = 0;
				} else if (keysdown[3] != 0) {
					shipimg = 2;
					shipx += (t_diff * SHIPSPEED);

					if (shipx >= 800 - img_player[shipimg]->w) shipx = 799.99993896484375f - img_player[shipimg]->w;
				} else shipimg = 1;

				// firing a bullet
				if (cooldown > 0) cooldown -= urate;

				if (keysdown[4] != 0 && cooldown <= 0) {
					createbullet(1, 64, shipx, shipy + img_player[shipimg]->h / 2, 4);
					createbullet(1, 64, shipx + img_player[shipimg]->w, shipy + img_player[shipimg]->h / 2, 4);
					cooldown = 500;
					playnoise(P_SHOT, rate, shipx + img_player[shipimg]->w / 2);
				}

				// handle speed changes
				int prev_rate = rate;

				if (keysdown[5] != 0 && (env->debug || speedbar > 0)) {
					rate = 0;
					speedbar -= t_diff * BARSPEED;

					if (speedbar < 0) speedbar = 0;
				} else if (keysdown[6] != 0) {
					rate = 2;
					speedbar += t_diff * BARSPEED;

					if (speedbar > 769) speedbar = 768.99993896484375f;
				} else
					rate = 1;

				if (rate < prev_rate) playnoise(S_DOWN, 1, 400);
				else if (rate > prev_rate) playnoise(S_UP, 1, 400);

				// player invulnerability timer
				if (invuln > 0) {
					invuln -= t_diff;

					if (invuln < 0) invuln = 0;
					else shipimg = 3;
				}

				// update scrolling background images etc
				//  background starfield
				bgpos = fmodf(bgpos + urate * SCROLLSPEED, 600);
				// strip updates
				//  this requires more work to create enemies etc
				strippos += 2 * urate * SCROLLSPEED;

				while (strippos >= 600) {
					strippos -= 128;

					if (stripindex < LVLSIZE - 6) {
						stripindex++;

						while (slist[spawnidx].when && slist[spawnidx].when <= stripindex) {
							createenemy(slist[spawnidx].type, slist[spawnidx].x, slist[spawnidx].y, slist[spawnidx].arg1, slist[spawnidx].arg2);

							// if an end-level boss spawned, play the boss music
							if (slist[spawnidx].type > 18) {
								Mix_HaltMusic();

								if (music_boss != NULL) Mix_PlayMusic(music_boss, -1);
							}

							spawnidx++;
						}
					}
				}

				// speed lines overlay
				overlaypos = fmodf(overlaypos + 4 * urate * SCROLLSPEED, 600);

				/* EXPLOSIONS */
				for (i = 0; i < SCREENEX; i++) {
					if (explo[i].type != 0) {
						explo[i].y += (urate * SCROLLSPEED);
						explo[i].timer += (urate * EXPLOSPEED);

						if (explo[i].timer >= 5) {
							// bit of a hack, but: if this was the Boss Explosion, it's victory
							//  and if it was the Player Explosion, game over!
							if (explo[i].type == 3)
								state = STATE_VICTORY;
							else if (explo[i].type == 1 && lives <= 0)
								state = STATE_GAMEOVER;
							else
								explo[i].type = 0;
						}
					}
				}

				/* BULLETS */
				for (i = 0; i < SCREENBUL; i++) {
					if (bullets[i].type != 0) {
						bullets[i].x += urate * SCROLLSPEED * bullets[i].dx;
						bullets[i].y += urate * SCROLLSPEED * bullets[i].dy;
						// bullet flies off screen
						int x = bullets[i].x;
						int y = bullets[i].y;

						if (y <= -img_shot[bullets[i].type - 1]->h || y >= 600 ||
							x <= -img_shot[bullets[i].type - 1]->w || x >= 800)
							bullets[i].type = 0;
						else if (bullets[i].type == 1) {
							// player bullet - check for collision against an enemy
							for (j = 0; j < SCREENE; j++) {
								if (elist[j].type != 0) {
									if (check_pixel_collision(x, y, elist[j].x, elist[j].y,
											img_shot[0], img_enemy[elist[j].type - 1])) {
										bullets[i].type = 0;
										elist[j].life--;
										break;
									}
								}
							}
						} else if (!invuln) {
							// enemy bullet - if player is not invuln. then check if a bullet hit them
							if (check_pixel_collision(x, y, shipx, shipy,
									img_shot[bullets[i].type - 1], img_player[shipimg])) {
								bullets[i].type = 0;
								invuln = 3000;
								int ctr_x = shipx + img_player[shipimg]->w / 2;
								createexplosion(1, ctr_x, shipy + img_player[shipimg]->h / 2);
								playnoise(P_DIE, rate, ctr_x);
								lives--;
							}
						}
					}
				}

				/* ENEMY UPDATES AND AI BEHAVIOR */
				for (i = 0; i < SCREENE; i++) {
					if (elist[i].type != 0) {
						switch (elist[i].type) {
						case 1:                      // stationary turret
							elist[i].y += SCROLLSPEED * 2 * urate;
							elist[i].timer += urate;

							if (elist[i].timer >= 1000) {
								elist[i].timer -= 1000;
								float ctr_x = elist[i].x + img_enemy[0]->w / 2;
								createbullet(2, 192, ctr_x, elist[i].y + img_enemy[0]->h, 2.5);
								playnoise(E_SHOT, rate, ctr_x);
							}

							break;

						case 2:                     // spinning shooting thing
							elist[i].y += SCROLLSPEED * urate;
							elist[i].timer += urate;

							if (elist[i].timer >= 250) {
								elist[i].timer -= 250;
								float ctr_x = elist[i].x + img_enemy[1]->w / 2;
								createbullet(3, elist[i].arg1, ctr_x, elist[i].y + img_enemy[1]->h / 2, 3);
								playnoise(E_SHOT, rate, ctr_x);
								// advance angle by increment
								// keep angle within limits
								elist[i].arg1 = (elist[i].arg1 + elist[i].arg2) & 0xFF;
							}

							break;

						case 3:							// single homing rocket launcher
							//							elist[i].y += SCROLLSPEED * urate;
							elist[i].x += SCROLLSPEED * 4 * urate * elist[i].arg2;

							if (!elist[i].timer &&
								((elist[i].x > elist[i].arg1 && elist[i].arg2 > 0) ||
									(elist[i].x < elist[i].arg1 && elist[i].arg2 < 0))) {
								elist[i].timer = 1;
								float ctr_x = elist[i].x + img_enemy[2]->w / 2;
								createenemy(14, ctr_x - img_enemy[12]->w / 2, elist[i].y + img_enemy[2]->h, 0, 16);
								playnoise(E_LAUNCH, rate, ctr_x);
							}

							break;

						case 4:                          // charger
							elist[i].y += SCROLLSPEED * 3 * urate;
							elist[i].timer += urate;

							if (elist[i].timer >= 175) {
								elist[i].timer -= 175;
								elist[i].arg1 ++;

								if (elist[i].arg1 < 7) {
									switch (elist[i].arg2) {
									case 2:
										j = 190;
										break;

									case 1:
									case 3:
										j = 191;
										break;

									case 0:
									case 4:
										j = 192;
										break;

									case 5:
									case 7:
										j = 193;
										break;

									case 6:
										j = 194;
										break;
									}

									elist[i].arg2 = (elist[i].arg2 + 1) % 8;
									float ctr_x = elist[i].x + img_enemy[3]->w / 2;
									createbullet(2, j, ctr_x, elist[i].y + img_enemy[3]->h, 5);
									playnoise(E_SHOT, rate, ctr_x);
								} else if (elist[i].arg1 > 9)
									elist[i].arg1 = 0;
							}

							break;

						case 5:                          // depth charge
						case 6:                          // depth charge
						case 7:                          // depth charge
							elist[i].y += SCROLLSPEED * urate;

							if (elist[i].y >= elist[i].arg1) {
								int ctr_x = elist[i].x + img_enemy[4]->w / 2;
								int ctr_y = elist[i].y + img_enemy[4]->h / 2;

								for (j = 0; j < 256; j += 32)
									createbullet(5, j, ctr_x, ctr_y, 5);

								// rather than set life to 0 (because that would give the player points...)
								createexplosion(4, ctr_x, ctr_y);
								playnoise(E_DIE, rate, ctr_x);
								elist[i].type = 0;
							} else if (elist[i].y + 50 >= elist[i].arg1) elist[i].type = 7;
							else if (elist[i].y + 150 >= elist[i].arg1) elist[i].type = 6;

							break;

						case 8:                          //side-shooting upwards rocketguy
							elist[i].y -= SCROLLSPEED * urate;
							elist[i].timer += urate;

							if (elist[i].timer >= 100) {
								elist[i].timer -= 100;

								if (elist[i].arg1 < 20) {
									for (j = 36; j < 89; j += 26) {
										createbullet(4, 128, elist[i].x, j + elist[i].y, 5);
										createbullet(4, 0, elist[i].x + img_enemy[7]->w, j + elist[i].y, 5);
									}

									playnoise(E_SHOT, rate, elist[i].x + img_enemy[7]->w / 2);
								}

								elist[i].arg1 = (elist[i].arg1 + 1) % 50;
							}

							break;

						case 9:                          //brick!
							elist[i].y += SCROLLSPEED * 2 * urate;
							break;

						case 10:                          //carrier: launches fighters
							if (elist[i].y < 0) {
								elist[i].y += SCROLLSPEED * urate;

								if (elist[i].y > 0)
									elist[i].y = 0;
							} else {
								elist[i].timer += urate;

								if (elist[i].timer >= 5000) {
									elist[i].timer -= 5000;
									createenemy(11, elist[i].x + (img_enemy[9]->w - img_enemy[10]->w) / 2, elist[i].y, 0, 0);
									playnoise(E_LAUNCH, rate, elist[i].x + img_enemy[9]->w / 2);
								}
							}

							break;

						case 11:                         //carrier fighter!
							elist[i].y += SCROLLSPEED * urate;

							if (elist[i].y >= 128) {
								elist[i].y += SCROLLSPEED * urate;

								if (elist[i].x < shipx) elist[i].x += SCROLLSPEED * urate;

								if (elist[i].x > shipx) elist[i].x -= SCROLLSPEED * urate;

								elist[i].timer += urate;

								if (elist[i].timer >= 350) {
									elist[i].timer -= 350;
									createbullet(5, 192, elist[i].x, elist[i].y + img_enemy[10]->h, 3);
									createbullet(5, 192, elist[i].x + img_enemy[10]->w, elist[i].y + img_enemy[10]->h, 3);
									playnoise(E_SHOT, rate, elist[i].x + img_enemy[10]->w / 2);
								}
							}

							break;

						case 12:                         //uberbomb.
							elist[i].y += urate * SCROLLSPEED;

							if (elist[i].y + img_enemy[11]->h / 2 >= 300) {
								speedbar = 0;
								createexplosion(5, 400, 300);
								playnoise(B_DIE, rate, 400);
								elist[i].type = 0;
							}

							break;

						case 13:
						case 14:
						case 15:              // homing rocket
							// movement
							elist[i].x += SCROLLSPEED * .0625 * urate * elist[i].arg1;
							elist[i].y += SCROLLSPEED * .0625 * urate * elist[i].arg2;
							// angle adjustments every 200 msec
							elist[i].timer += urate;

							if (elist[i].timer > 200) {
								elist[i].timer -= 200;
								// rotate missile towards player
								//  determine angle to player
								double angle = atan2(
										(shipy + img_player[shipimg]->h / 2) - (elist[i].y + img_enemy[12]->h / 2),
										(shipx + img_player[shipimg]->w / 2) - (elist[i].x + img_enemy[12]->w / 2));

								// only adjust speed if angle is within certain ranges
								if (angle >= M_PI / 4 && angle <= 3 * M_PI / 4) {
									// adjust speeds
									elist[i].arg1 += 8 * cos(angle);
									elist[i].arg2 += 8 * sin(angle);

									// fix graphic
									if (angle < 3 * M_PI / 8) elist[i].type = 15;
									else if (angle > 5 * M_PI / 8) elist[i].type = 13;
									else elist[i].type = 14;
								}
							}

							break;

						case 16:                // laser boss helper
							elist[i].arg1 -= urate;

							if (elist[i].arg1 <= 0)
								elist[i].type = 0;
							else {
								elist[i].timer += urate;

								if (elist[i].timer >= 50) {
									elist[i].timer -= 50;
									int ctr_x = elist[i].x + img_enemy[15]->w / 2;
									createbullet(5, elist[i].arg2, ctr_x, elist[i].y + img_enemy[15]->h / 2, 5);
									playnoise(E_SHOT, rate, ctr_x);
								}
							}

							break;

						case 17:
						case 18:
						case 19:             // space station!
							boss_timer += urate;

							if (boss_timer >= 400) {    //animate
								boss_timer -= 400;
								elist[i].type++;

								if (elist[i].type == 20) elist[i].type = 17;
							}

							// move onto screen
							if (elist[i].y < 0) {
								elist[i].y += SCROLLSPEED * urate;

								if (elist[i].y > 0)
									elist[i].y = 0;
							} else {
								// do all battle actions here.
								elist[i].timer += urate;

								if (elist[i].timer >= 100) {
									elist[i].timer -= 100;
									// roll over on action frame
									//  (the first action intentionally skipped via off-by-one)
									elist[i].arg2++;

									if (elist[i].arg2 > 74) {
										elist[i].arg2 -= 75;
										elist[i].arg1 = (elist[i].arg1 + 1) % 5;
									}

									// now actually TAKE the action
									int ctr_x = elist[i].x + img_enemy[17]->w / 2;
									int ctr_y = elist[i].y + img_enemy[17]->h / 2;

									switch (elist[i].arg1) {
									case 0:				// time draining bomb
										if (elist[i].arg2 == 0) {
											createenemy(12, ctr_x - img_enemy[11]->w / 2, ctr_y, 0, 0);
											playnoise(E_LAUNCH, rate, ctr_x);
										}

										break;

									case 1:				// machine gun
										if (elist[i].arg2 % 7 != 0 && elist[i].arg2 < 50) {
											j = 128.0 / M_PI * atan2(ctr_y - (shipy + img_player[shipimg]->h / 2), (shipx + img_player[shipimg]->w / 2) - ctr_x);
											j &= 0xFF;
											createbullet(5, j, ctr_x, ctr_y, 5);
											playnoise(E_SHOT, rate, ctr_x);
										}

										break;

									case 2:				// brick walls
										if (elist[i].arg2 % 50 == 5) {
											createenemy(9, 0, -32, 0, 0);
											createenemy(9, 84, -32, 0, 0);
											createenemy(9, 184, -32, 0, 0);
											createenemy(9, 584, -32, 0, 0);
											createenemy(9, 684, -32, 0, 0);
											createenemy(9, 768, -32, 0, 0);
											playnoise(E_LAUNCH, rate, ctr_x);
										}

										break;

									case 3:				// rocket salvo
										if (elist[i].arg2 % 25 == 0) {
											for (j = 0; j <= 4; j++)
												createenemy(14, ctr_x + (32 * (j - 2)) - img_enemy[12]->w / 2, ctr_y, 24 * (j - 2), 16);

											playnoise(E_LAUNCH, rate, ctr_x);
										}

										break;

									case 4:				// two spinners
										if (elist[i].arg2 == 5) {
											createenemy(2, 184, -32, 0, -13);
											createenemy(2, 584, -32, 128, 13);
//											createenemy(5, 384, -32, 484, 0);
											playnoise(E_LAUNCH, rate, ctr_x);
										}

										break;
									}
								}
							}

							break;

						case 20:                // shotgun boss
							elist[i].timer += urate;

							if (elist[i].timer >= 4000) {
								elist[i].timer -= 4000;
								// determine gap in shot
								int gap;

								switch (elist[i].arg1) {
								case 0:
									gap = 170;
									break;

								case 1:
								case 7:
									gap = 181;
									break;

								case 2:
								case 6:
									gap = 192;
									break;

								case 3:
								case 5:
									gap = 203;
									break;

								default:
									gap = 214;
									break;
								}

								// advance state w/ rollover
								elist[i].arg1 = (elist[i].arg1 + 1) % 8;
								// create bullet spread
								int ctr_x = elist[i].x + img_enemy[19]->w / 2;

								for (j = 129; j < gap - 4; j ++)
									createbullet(2, j, ctr_x, elist[i].y, 4);

								for (j = gap + 4; j <= 255; j++)
									createbullet(2, j, ctr_x, elist[i].y, 4);

								// play a bunch of shooty noises
								playnoise(E_SHOT, rate, 400);
								playnoise(E_LAUNCH, rate, 400);
							}

							break;

						case 21:               //dcboss
							elist[i].x += urate * SCROLLSPEED * elist[i].arg1;

							if (elist[i].x <= 128) elist[i].arg1 = 1;
							else if (elist[i].x + img_enemy[20]->w >= 672) elist[i].arg1 = -1;

							elist[i].timer += urate;

							if (elist[i].timer > 5000) {
								elist[i].timer -= 5000;
								int ctr_x = elist[i].x + img_enemy[20]->w / 2;
								createenemy(5, ctr_x - img_enemy[4]->w / 2, elist[i].y + img_enemy[20]->h, shipy + img_player[shipimg]->h / 2 - img_enemy[4]->h / 2, 0);
								playnoise(E_LAUNCH, rate, ctr_x);
							}

							break;

						case 22:               //laser boss
							elist[i].timer += urate;

							if (elist[i].timer >= 1000) {
								elist[i].timer -= 1000;
								j = 128.0 / M_PI * atan2(
										(elist[i].y + img_enemy[21]->h / 2) - (shipy + img_player[shipimg]->h / 2),
										(shipx + img_player[shipimg]->w / 2) - (elist[i].x + img_enemy[21]->w / 2));
								createenemy(16, elist[i].x + (img_enemy[21]->w - img_enemy[15]->w) / 2, elist[i].y + (img_enemy[21]->h - img_enemy[15]->h) / 2, 900, j & 0xFF);
							}

							break;

						case 23:               //rocket boss
							elist[i].x += SCROLLSPEED * urate * elist[i].arg1;

							if (elist[i].x <= 250) elist[i].arg1 = 1;
							else if (elist[i].x + img_enemy[22]->w >= 550) elist[i].arg1 = -1;

							elist[i].timer += urate;

							if (elist[i].timer >= 1500) {
								elist[i].timer -= 1500;
								createenemy(13, elist[i].x, elist[i].y + 48, -32, 16);
								createenemy(14, elist[i].x - 4 + 64, elist[i].y + 64, 0, 16);
								createenemy(15, elist[i].x - 8 + 128, elist[i].y + 48, 32, 16);
								//								int ctr_x = elist[i].x + (64 << 5);
								playnoise(E_LAUNCH, rate, elist[i].x + 64);
							}

							break;

						case 24:               //spinner boss
							elist[i].x += urate * SCROLLSPEED * elist[i].arg1;

							if (elist[i].x <= 128) elist[i].arg1 = 1;
							else if (elist[i].x + img_enemy[23]->w >= 672) elist[i].arg1 = -1;

							elist[i].timer += urate;

							if (elist[i].timer >= 5000) {
								elist[i].timer -= 5000;
								int ctr_x = elist[i].x + 64;
								createenemy(2, ctr_x - img_enemy[1]->w / 2, elist[i].y + 32, 192, elist[i].arg1 * 11);
								playnoise(E_LAUNCH, rate, ctr_x);
							}

							break;
						}

						// if enemy didn't die before now...
						if (elist[i].type) {
							// enemy has flown off-screen, remove it
							if (elist[i].y + img_enemy[elist[i].type - 1]->h <= 0 || elist[i].y >= 600 ||
								elist[i].x + img_enemy[elist[i].type - 1]->w <= 0 || elist[i].x >= 800)
								elist[i].type = 0;
							// enemy was killed this frame
							else if (elist[i].life <= 0) {
								*score += scores[elist[i].type - 1];
								int ctr_x = elist[i].x + img_enemy[elist[i].type - 1]->w / 2;
								int ctr_y = elist[i].y + img_enemy[elist[i].type - 1]->h / 2;
								createexplosion(2,
									ctr_x,
									ctr_y
								);

								// boss explosion
								if (elist[i].type > 16) {
									if (elist[i].type < 20)
										// Just for fun, make one of these big blue bombs go off.
										createexplosion(5, ctr_x, ctr_y);

									// some other scattered explosions for bosses
									createexplosion(1, ctr_x - 32, ctr_y - 32);
									createexplosion(3, ctr_x + 32, ctr_y - 32);
									createexplosion(3, ctr_x - 32, ctr_y + 32);
									createexplosion(1, ctr_x + 32, ctr_y + 32);
									playnoise(B_DIE, rate, ctr_x);
								} else
									playnoise(E_DIE, rate, ctr_x);

								elist[i].type = 0;
							}
							// collision with player
							else if (invuln == 0 && check_pixel_collision(elist[i].x, elist[i].y, shipx, shipy,
									img_enemy[elist[i].type - 1], img_player[shipimg])) {
								elist[i].life -= 5;
								invuln = 5000;
								int ctr_x = shipx + img_player[shipimg]->w / 2;
								createexplosion(1, ctr_x, shipy + img_player[shipimg]->h / 2);
								playnoise(P_DIE, rate, ctr_x);
								lives--;
							}
						}
					}
				}
			}

			/* draw */
			// Background
			bgrect.y = bgpos;
			SDL_BlitSurface(bg, NULL, env->screen, &bgrect);
			bgrect.y -= 600;
			SDL_BlitSurface(bg, NULL, env->screen, &bgrect);
			// Up to six "strips" (bg image slices)
			striprect.y = strippos;

			for (i = 0; i < 6; i++) {
				SDL_BlitSurface(strips[stripslist[stripindex + i]], NULL, env->screen, &striprect);
				striprect.y -= 128;
			}

			/* Speed lines */
			bgrect.y = overlaypos;
			SDL_BlitSurface(img_overlay[(int)rate], NULL, env->screen, &bgrect);
			bgrect.y -= 600;
			SDL_BlitSurface(img_overlay[(int)rate], NULL, env->screen, &bgrect);

			/* Debug grid */
			if (env->debug) {
				SDL_Rect tmprect = {};
				tmprect.w = 1;
				tmprect.h = 600;

				for (tmprect.x = 100; tmprect.x < 800; tmprect.x += 100)
					SDL_FillRect(env->screen, &tmprect, color_blue);
			}

			// all enemies
			for (i = 0; i < SCREENE; i++) {
				if (elist[i].type != 0) {
					SDL_Rect tmprect = {};
					tmprect.w = img_enemy[elist[i].type - 1]->w;
					tmprect.h = img_enemy[elist[i].type - 1]->h;
					tmprect.x = elist[i].x;
					tmprect.y = elist[i].y;
					SDL_BlitSurface(img_enemy[elist[i].type - 1], NULL, env->screen, &tmprect);
				}
			}

			// player sprite
			shiprect.x = shipx;
			shiprect.y = shipy;
			SDL_BlitSurface(img_player[shipimg], NULL, env->screen, &shiprect);

			// all bullets
			for (i = 0; i < SCREENBUL; i++) {
				if (bullets[i].type != 0) {
					bulletrect.h = img_shot[bullets[i].type - 1]->w;
					bulletrect.x = bullets[i].x;
					bulletrect.y = bullets[i].y;
					SDL_BlitSurface(img_shot[bullets[i].type - 1], NULL, env->screen, &bulletrect);
				}
			}

			// all explosions
			for (i = 0; i < SCREENEX; i++) {
				// don't show off-screen ones
				if (explo[i].type != 0 && explo[i].y < 600) {
					int frame = (explo[i].type - 1) * 5 + explo[i].timer;
					SDL_Rect tmprect = {};
					tmprect.w = img_explo[frame]->w;
					tmprect.h = img_explo[frame]->h;
					tmprect.x = explo[i].x;
					tmprect.y = explo[i].y;
					SDL_BlitSurface(img_explo[frame], NULL, env->screen, &tmprect);
				}
			}

			/* HUD items */
			if (gpause)
				SDL_BlitSurface(img_pause, NULL, env->screen, &pauserect);

			// lives
			SDL_BlitSurface(img_player[1], NULL, env->screen, NULL);
			blit_number(env->screen, 80, 16, lives - 1);
			// score
			blit_number(env->screen, 768, 16, *score);
			// time meter
			indicrect.w = speedbar;
			SDL_FillRect(env->screen, &indicrect, color_blue);

			if (env->debug) {
				blit_number(env->screen, 80, 64, stripindex);
				blit_number(env->screen, 768, 64, spawnidx);
			}

			SDL_Flip(env->screen);
		}

		SDL_Delay(1);
	} while (state == STATE_GAME);

	/* *********************************************************************** */
	/* shut down */
	if (level < 6) {
		for (i = 0; i < 42; i++)
			SDL_FreeSurface(strips[i]);
	}

	Mix_HaltChannel(-1);
	Mix_FreeMusic(music);
	return state;
}

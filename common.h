/* common.h - shared items and globals */
#ifndef COMMON_H_
#define COMMON_H_

#include <SDL/SDL.h>
#ifdef __APPLE__
#include "SDL_mixer/SDL_mixer.h"
#include "SDL_image/SDL_image.h"
#else
#include <SDL/SDL_mixer.h>
#include <SDL/SDL_image.h>
#endif

// Major Gamestates
#define STATE_ERROR -1
#define STATE_QUIT 0
#define STATE_UI 1
#define STATE_PLAY 2

// this is not specified anywhere, so...
#define MIX_DEFAULT_BUFFERS "1024"

// helpers
#define KEY_UP keys[0]
#define KEY_DOWN keys[1]
#define KEY_LEFT keys[2]
#define KEY_RIGHT keys[3]
#define KEY_FIRE keys[4]
#define KEY_SPEED_DOWN keys[5]
#define KEY_SPEED_UP keys[6]
#define KEY_START keys[7]
#define KEY_PAUSE keys[8]
#define KEY_QUIT keys[9]

/* Struct declaration of common game objects
    These are shared across multiple major states at once */
struct env_t {
	// Screen surface and mouse cursor image
	SDL_Surface * screen;
	SDL_Surface * cursor;

	// Music and SFX enabled (checkboxes)
	int sfx_works, mus_works;
	int sfx_on, mus_on;

	// Key bindings
	SDLKey keys[10];
	// Joystick buttons
	int buttons[4];

	// High scores
	int hs_score[3];
	int hs_red[3];
	int hs_green[3];
	int hs_blue[3];

	// cheat keys enabled?
	int debug;
};

/* Init and shutdown things needed (below) */
int init_common(void);
void free_common(void);

/* common functions used across all modules */
SDL_Surface * load_image(const char * path, int alpha);
Mix_Music * load_music(const char * path, int autoplay);
Mix_Chunk * load_sample(const char * path);

void blit_number(SDL_Surface * dest, unsigned int x, unsigned int y, int number);

#endif

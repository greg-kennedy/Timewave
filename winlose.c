/* menu.c - menu */

#include "winlose.h"

#include "common.h"

#include "meta_play.h"

/* ****************************** */
// Game Over
int state_gameover(struct env_t * env, const int score)
{
	/* Load the resources for this screen */
	SDL_Surface * bg = load_image("img/ui/gameover.png", 0);

	if (bg == NULL) return STATE_ERROR;

	Mix_Music * music = NULL;

	if (env->mus_works && env->mus_on)
		music = load_music("audio/lose.mod", 1);

	/* Require 1-second delay before registering keypress on this scene */
	const Uint32 wl_debounce = SDL_GetTicks() + 1000;
	/* One-time draw the screen */
	int dirty = 1;
	/* Event loop */
	int state = STATE_GAMEOVER;

	do {
		/* Check for events */
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_VIDEOEXPOSE:
				dirty = 1;
				break;

			case SDL_KEYUP:
				if ((event.key.keysym.sym == env->KEY_FIRE || event.key.keysym.sym == env->KEY_QUIT || event.key.keysym.sym == env->KEY_START) && wl_debounce < SDL_GetTicks())
					state = STATE_PLAY_END;

				break;

			case SDL_JOYBUTTONUP:
				if (event.jbutton.button == env->buttons[0] && wl_debounce < SDL_GetTicks())
					state = STATE_PLAY_END;

				break;

			case SDL_MOUSEBUTTONUP:
				state = STATE_PLAY_END;
				break;

			case SDL_QUIT:
				state = STATE_QUIT;
			}
		}

		if (dirty) {
			SDL_BlitSurface(bg, NULL, env->screen, NULL);
			blit_number(env->screen, 450, 375, score);
			SDL_Flip(env->screen);
			dirty = 0;
		}

		SDL_Delay(1);
	} while (state == STATE_GAMEOVER);

	Mix_FreeMusic(music);
	SDL_FreeSurface(bg);
	return state;
}

int state_victory(struct env_t * env, const int score)
{
	/* Load the resources for this screen */
	SDL_Surface * bg = load_image("img/ui/victory.png", 0);

	if (bg == NULL) return STATE_ERROR;

	Mix_Music * music = NULL;

	if (env->mus_works && env->mus_on)
		music = load_music("audio/win.mod", 1);

	/* Require 1-second delay before registering keypress on this scene */
	const Uint32 wl_debounce = SDL_GetTicks() + 1000;
	/* One-time draw the screen */
	int dirty = 1;
	/* Event loop */
	int state = STATE_VICTORY;

	do {
		/* Check for events */
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_VIDEOEXPOSE:
				dirty = 1;
				break;

			case SDL_JOYBUTTONUP:
				if (event.jbutton.button == env->buttons[0] && wl_debounce < SDL_GetTicks())
					state = STATE_GAME_NEXT;

				break;

			case SDL_KEYUP:
				if ((event.key.keysym.sym == env->KEY_FIRE || event.key.keysym.sym == env->KEY_QUIT || event.key.keysym.sym == env->KEY_START) && wl_debounce < SDL_GetTicks())
					state = STATE_GAME_NEXT;

				break;

			case SDL_MOUSEBUTTONUP:
				state = STATE_GAME_NEXT;
				break;

			case SDL_QUIT:
				state = STATE_QUIT;
			}
		}

		if (dirty) {
			SDL_BlitSurface(bg, NULL, env->screen, NULL);
			blit_number(env->screen, 528, 513, score);
			SDL_Flip(env->screen);
			dirty = 0;
		}

		SDL_Delay(1);
	} while (state == STATE_VICTORY);

	Mix_FreeMusic(music);
	SDL_FreeSurface(bg);
	return state;
}

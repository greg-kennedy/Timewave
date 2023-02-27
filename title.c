/* title.c - title screen */

#include "title.h"

#include "common.h"

#include "meta_ui.h"

/* *************************************************** */
/* TITLE SCREEN */
int state_title(struct env_t * env, const int arcade_mode)
{
	/* Load the resources for this screen */
	SDL_Surface * bg = load_image("img/ui/title.png", 0);

	if (bg == NULL)
		return STATE_ERROR;

	// Five seconds on this screen in Arcade Mode
	const Uint32 arcade_ticks = SDL_GetTicks() + 5000;
	/* One-time draw the screen */
	int dirty = 1;
	/* Event loop */
	int state = STATE_TITLE;

	do {
		SDL_Event event;

		if (! arcade_mode) {
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_VIDEOEXPOSE:
					dirty = 1;
					break;

				case SDL_KEYUP:
				case SDL_MOUSEBUTTONUP:
				case SDL_JOYBUTTONUP:
					state = STATE_MENU;
					break;

				case SDL_QUIT:
					state = STATE_QUIT;
				}
			}
		} else {
			if (arcade_ticks < SDL_GetTicks())
				state = STATE_HS;

			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_VIDEOEXPOSE:
					dirty = 1;
					break;

				case SDL_KEYUP:
					if (event.key.keysym.sym == env->KEY_START)
						state = STATE_PLAY;
					else if (event.key.keysym.sym == env->KEY_QUIT)
						state = STATE_QUIT;

					break;

				case SDL_MOUSEBUTTONUP:
				case SDL_JOYBUTTONUP:
					state = STATE_PLAY;
					break;

				case SDL_QUIT:
					state = STATE_QUIT;
				}
			}
		}

		/* draw */
		if (dirty) {
			SDL_BlitSurface(bg, NULL, env->screen, NULL);
			SDL_Flip(env->screen);
			dirty = 0;
		}

		SDL_Delay(1);
	} while (state == STATE_TITLE);

	SDL_FreeSurface(bg);
	return state;
}

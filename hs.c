/* menu.c - menu */

#include "hs.h"

#include "common.h"

#include "meta_ui.h"

/* *************************************************** */
/* HIGH SCORE DISPLAY */
int state_hs(struct env_t* env, const int arcade_mode)
{
	/* Load the resources for this screen */
	SDL_Surface *bg = load_image("img/ui/hs.png", 0);
	if (bg == NULL)
		return STATE_ERROR;

	// generate the three color values for the banner
	Uint32 color[3];
	for (int i = 0; i < 3; i ++)
		color[i] = SDL_MapRGB(env->screen->format, env->hs_red[i], env->hs_green[i], env->hs_blue[i]);

	// banner squares
	SDL_Rect colobox;
	colobox.w = 64;
	colobox.h = 64;
	colobox.x = 128;

	// Five seconds on this screen in Arcade Mode
	const Uint32 arcade_ticks = SDL_GetTicks() + 5000;

	/* One-time draw the screen */
	int dirty = 1;

	/* Event loop */
	int state = STATE_HS;
	do
	{
		SDL_Event event;
		if (! arcade_mode) {
			while (SDL_PollEvent (&event))
			{
				switch (event.type)
				{
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

			while (SDL_PollEvent(&event))
			{
				switch (event.type)
				{
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

		if (dirty) {
			// background image
			SDL_BlitSurface(bg, NULL, env->screen, NULL);

			// three color banners
			colobox.y = 128;
			SDL_FillRect(env->screen, &colobox, color[0]);
			colobox.y = 224;
			SDL_FillRect(env->screen, &colobox, color[1]);
			colobox.y = 320;
			SDL_FillRect(env->screen, &colobox, color[2]);

			// high score numbers
			blit_number(env->screen, 500, 142, env->hs_score[0]);
			blit_number(env->screen, 500, 238, env->hs_score[1]);
			blit_number(env->screen, 500, 334, env->hs_score[2]);

			SDL_Flip(env->screen);

			dirty = 0;
		}

		SDL_Delay(1);

	} while (state == STATE_HS);

	SDL_FreeSurface(bg);

	return state;
}

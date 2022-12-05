/* keyconf.c - keyconf */

#include "keyconf.h"

#include "common.h"

#include "meta_ui.h"

/* *************************************************** */
/* KEY CONFIG */
int state_keyconf(struct env_t* env)
{
	/* Load the resources for this screen */
	SDL_Surface* bg = load_image("img/ui/keyconf.png", 0);
	if (bg == NULL)
		return STATE_ERROR;

	// rectangle for keys set so far
	SDL_Rect lightrect;
	lightrect.x = 200;
	lightrect.w = 80;
	lightrect.y = 200;
	lightrect.h = 0;

	// rectangle for key being set
	SDL_Rect currect;
	currect.x = 200;
	currect.w = 80;
	currect.y = 200;
	currect.h = 30;

	// three colors we will be using
	const Uint32 color_red = SDL_MapRGB(env->screen->format, 255, 0, 0);
	const Uint32 color_yellow = SDL_MapRGB(env->screen->format, 255, 255, 0);
	const Uint32 color_green = SDL_MapRGB(env->screen->format, 0, 255, 0);

	Uint32 lightcolor = color_red;

	int current_configging_key = 0;
	SDLKey current_held_key = (SDLKey)0;
	Uint32 key_ticks = 0;

	/* One-time draw the screen */
	int dirty = 1;

	/* Event loop */
	int state = STATE_KEYCONF;
	do
	{
 		/* Check for events */
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
				case SDL_VIDEOEXPOSE:
					dirty = 1;
					break;
				case SDL_KEYDOWN:
					if (current_held_key == 0)
					{
						// Pressed a new key: record it and set the timeout to one second
						current_held_key = event.key.keysym.sym;
						lightcolor = color_yellow;
						key_ticks = SDL_GetTicks() + 1000;
					} else {
						// Pressed multiple keys at once, cancel what we were doing
						current_held_key = (SDLKey)0;
						lightcolor = color_red;
					}
					dirty = 1;
					break;
				case SDL_KEYUP:
					if (current_configging_key == 10) {
						// if we did ALL the keys, time to go back to main menu.
						state = STATE_MENU;
					} else if (current_held_key != 0) {
						// Key release changes the light back to red
						lightcolor = color_red;
						current_held_key = (SDLKey)0;

						dirty = 1;
					}
					break;
				case SDL_QUIT:
					state = STATE_QUIT;
			}
		}

		/* Check for keypress held long enough to lock-in */
		if (current_held_key != 0 && key_ticks < SDL_GetTicks())
		{
			env->keys[current_configging_key] = current_held_key;
			current_held_key = (SDLKey)0;
			current_configging_key++;

			lightrect.h += 30;
			currect.y += 30;
			lightcolor = color_red;

			dirty = 1;
		}

		if (dirty)
		{
			// draw the background
			SDL_BlitSurface(bg, NULL, env->screen, NULL);

			// draw the lights for all keys we've done already
			if (current_configging_key > 0)
				SDL_FillRect(env->screen, &lightrect, color_green);

			// draw the light for the key we're working on
			if (current_configging_key != 10)
				SDL_FillRect(env->screen, &currect, lightcolor);

			SDL_Flip(env->screen);
			
			dirty = 0;
		}

		SDL_Delay(1);

	} while (state == STATE_KEYCONF);

	SDL_FreeSurface(bg);

	return state;
}

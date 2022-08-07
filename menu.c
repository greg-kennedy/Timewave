/* menu.c - menu */
#include "menu.h"

#include "common.h"
#include "meta_ui.h"

/* *************************************************** */
/* MAIN MENU */
int state_menu(struct env_t* env, Mix_Music* music)
{
	/* Load the resources for this screen */
	SDL_Surface* bg = load_image("img/ui/menu.png", 0);
	if (bg == NULL)
		return STATE_ERROR;

	SDL_Surface* checkbox = load_image("img/ui/checked.png", 0);
	if (checkbox == NULL)
	{
		SDL_FreeSurface(bg);
		return STATE_ERROR;
	}

	SDL_Rect cboxrect;
	cboxrect.x = 559;

	// mouse state
	int mx, my;
	SDL_GetMouseState(&mx, &my);

	/* One-time draw the screen */
	int dirty = 1;

	/* Event loop */
	int state = STATE_MENU;
	do
	{
		/* read events */
		SDL_Event event;
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_VIDEOEXPOSE:
					dirty = 1;
					break;
				case SDL_KEYUP:
					if (event.key.keysym.sym == env->KEY_QUIT)
						state = STATE_QUIT;
					else
						state = STATE_PLAY;
					break;
				case SDL_JOYBUTTONUP:
					if (event.jbutton.which == 0) {
						if (event.jbutton.button == 0)
							state = STATE_PLAY;
						else if (event.jbutton.button == 1)
							state = STATE_HS;
					}
					break;
				case SDL_MOUSEBUTTONUP:
					mx = event.button.x;
					my = event.button.y;
					if (mx > 98 && mx < 299)
					{
						if (my > 137 && my < 206)
							state = STATE_PLAY;
						if (my > 314 && my < 383)
							state = STATE_HS;
						if (my > 486 && my < 555)
							state = STATE_QUIT;
					}
					else if (mx > 550 && mx < 692)
					{
						if (my > 312 && my < 331)
						{
							if (env->mus_on == 0)
							{
								env->mus_on = 1;
								Mix_PlayMusic(music, -1);
							} else {
								env->mus_on = 0;
								if (Mix_PlayingMusic()) Mix_HaltMusic();
							}
						}
						else if (event.button.y > 369 && event.button.y < 388)
						{
							env->sfx_on = ! env->sfx_on;
						}
						else if (my > 410 && my < 468)
						{
							state = STATE_KEYCONF;
						}
					}
					dirty = 1;
					break;
				case SDL_MOUSEMOTION:
					mx = event.motion.x;
					my = event.motion.y;
					dirty = 1;
					break;
				case SDL_QUIT:
					state = STATE_QUIT;
			}
		}

		/* draw screen */
		if (dirty)
		{
			SDL_BlitSurface(bg, NULL, env->screen, NULL);

			// checkboxes
			if (env->mus_on)
			{
				cboxrect.y = 311;
				SDL_BlitSurface(checkbox, NULL, env->screen, &cboxrect);
			}
			if (env->sfx_on)
			{
				cboxrect.y = 370;
				SDL_BlitSurface(checkbox, NULL, env->screen, &cboxrect);
			}

			// mouse cursor
			SDL_Rect mcrect;
			mcrect.x = mx;
			mcrect.y = my;
			SDL_BlitSurface(env->cursor, NULL, env->screen, &mcrect);

			SDL_Flip(env->screen);

			dirty = 0;
		}

		SDL_Delay(1);

	} while (state == STATE_MENU);

	SDL_FreeSurface(checkbox);
	SDL_FreeSurface(bg);

	return state;
}

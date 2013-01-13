/* title.c - title screen */

#include "title.h"

extern int newjoy;

void inittitle()
{
	SDL_Surface *surf=NULL;
	surf=IMG_Load("img/ui/title.png");
	tempsurf = SDL_DisplayFormat(surf);
	SDL_FreeSurface(surf);

	/* This is the music to play. */
	if (muson==1 && (!arcade_mode || music == NULL)) {
		music = Mix_LoadMUS("audio/title.xm");
		Mix_PlayMusic(music, -1);
	}
	newjoy=0;

	tscreen_arcade_ticks = SDL_GetTicks() + 5000;
}

void destroytitle()
{
	SDL_FreeSurface(tempsurf);
}

void drawtitle()
{
	SDL_BlitSurface(tempsurf,NULL,screen,NULL);
	/* Make sure everything is displayed on screen */
	SDL_Flip (screen);
	/* Don't run too fast */
	SDL_Delay (1);
}

int handletitle()
{
	SDL_Event event;
	int retval=0;

	if (joysticks>0)
	{
		if (SDL_JoystickGetButton(joy,0)) {
			if (newjoy) {
				if (arcade_mode)
					gamestate=1;
				else
					gamestate=3;
			}
		} else {
			newjoy=1;
		}
	}

	if (!arcade_mode)
	{
		/* Check for events */
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_KEYUP:
					gamestate=1;
					break;
				case SDL_MOUSEBUTTONUP:
					gamestate=1;
					break;
				case SDL_QUIT:
					retval = 1;
					break;
				default:
					break;
			}
		}
	} else {
		if (tscreen_arcade_ticks < SDL_GetTicks())
			gamestate = 2;

		/* Check for events */
		while (SDL_PollEvent (&event))
		{
			switch (event.type)
			{
				case SDL_KEYUP:
					if (event.key.keysym.sym == KEY_START)
						gamestate=3;
					else if (event.key.keysym.sym == KEY_QUIT)
						retval = 1;
					break;
				case SDL_QUIT:
					retval = 1;
					break;
				default:
					break;
			}
		}
	}

	return retval;
}

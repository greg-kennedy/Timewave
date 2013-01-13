/* keyconf.c - keyconf */

#include "keyconf.h"

SDL_Rect lightrect,currect;

int current_configging_key;
SDLKey current_held_key;
Uint32 lightcolor, greenlight, key_ticks;

void initkeyconf()
{
     SDL_Surface *surf=NULL;
     surf=IMG_Load("img/ui/keyconf.png");
     tempsurf = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);

	lightrect.x = 200;
	currect.x = 200;
	lightrect.w = 80;
	currect.w = 80;
	lightrect.y = 200;
	lightrect.h = 0;
	currect.y = 200;
	currect.h = 30;

	greenlight = SDL_MapRGB(screen->format,0,255,0);

	lightcolor = SDL_MapRGB(screen->format,255,0,0);

	current_configging_key = 0;
	key_ticks = 0;
}

void destroykeyconf()
{
	 SDL_FreeSurface(tempsurf);
}

void drawkeyconf()
{
	SDL_BlitSurface(tempsurf,NULL,screen,NULL);

	SDL_FillRect(screen,&lightrect,greenlight);
	if (current_configging_key != 10)
	SDL_FillRect(screen,&currect,lightcolor);

    /* Make sure everything is displayed on screen */
    SDL_Flip (screen);
    /* Don't run too fast */
    SDL_Delay (1);
}

int handlekeyconf()
{
    SDL_Event event;
    int retval=0;

	if (key_ticks < SDL_GetTicks() && key_ticks != 0)
	{
		key_ticks = 0;
		keys[current_configging_key] = current_held_key;
		current_configging_key++;
		lightrect.h += 30;
		currect.y += 30;
		lightcolor = SDL_MapRGB(screen->format,255,0,0);
	}

    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
        case SDL_KEYDOWN:
		current_held_key = event.key.keysym.sym;
		lightcolor = SDL_MapRGB(screen->format,255,255,0);
		key_ticks = SDL_GetTicks()+1000;
            break;
        case SDL_KEYUP:
		lightcolor = SDL_MapRGB(screen->format,255,0,0);
		if (current_configging_key == 10) gamestate = 1;
		key_ticks = 0;
            break;
        case SDL_QUIT:
            retval = 1;
            break;
        default:
            break;
        }
    }
    
    return retval;
}

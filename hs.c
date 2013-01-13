/* menu.c - menu */

#include "hs.h"

extern int tscore[3];
extern int red[3];
extern int green[3];
extern int blue[3];

int newjoy;

void iniths()
{
     SDL_Surface *surf=NULL;
     surf=IMG_Load("img/ui/hs.png");
     tempsurf = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);
     newjoy=0;

	hs_arcade_ticks = SDL_GetTicks() + 5000;
}

void destroyhs()
{
	 SDL_FreeSurface(tempsurf);
}

void drawhs()
{
     int i;
     SDL_Rect colobox;
     long tempscore;
     SDL_Rect scorerect, tempscorerect;

	SDL_BlitSurface(tempsurf,NULL,screen,NULL);

	scorerect.h=32;
	scorerect.w=32;
	tempscorerect.h=32;
	tempscorerect.w=32;
	tempscorerect.y=0;

     colobox.w=64;
     colobox.h=64;
     colobox.x=128;
     colobox.y=128;
     SDL_FillRect(screen,&colobox,SDL_MapRGB(screen->format,red[0],
        green[0],blue[0]));
     colobox.y=224;
     SDL_FillRect(screen,&colobox,SDL_MapRGB(screen->format,red[1],
        green[1],blue[1]));
     colobox.y=320;
     SDL_FillRect(screen,&colobox,SDL_MapRGB(screen->format,red[2],
        green[2],blue[2]));

    scorerect.y=142;
	i=500;
	tempscore=tscore[0];
	while (tempscore > 0)
	{
          i=i-32;
          tempscorerect.x=32*(tempscore % 10);
          scorerect.x=i;
          SDL_BlitSurface(numbers,&tempscorerect,screen,&scorerect);
          tempscore=(long)(tempscore/10);
    }

    scorerect.y=238;
	i=500;
	tempscore=tscore[1];
	while (tempscore > 0)
	{
          i=i-32;
          tempscorerect.x=32*(tempscore % 10);
          scorerect.x=i;
          SDL_BlitSurface(numbers,&tempscorerect,screen,&scorerect);
          tempscore=(long)(tempscore/10);
    }

    scorerect.y=334;
	i=500;
	tempscore=tscore[2];
	while (tempscore > 0)
	{
          i=i-32;
          tempscorerect.x=32*(tempscore % 10);
          scorerect.x=i;
          SDL_BlitSurface(numbers,&tempscorerect,screen,&scorerect);
          tempscore=(long)(tempscore/10);
    }

    /* Make sure everything is displayed on screen */
    SDL_Flip (screen);
    /* Don't run too fast */
    SDL_Delay (1);
}

int handlehs()
{
    SDL_Event event;
    int retval=0;

    if (joysticks>0)
    {
       if (SDL_JoystickGetButton(joy,0)) {
         if (newjoy) { if (arcade_mode) gamestate=1; else gamestate=3; }
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
  if (hs_arcade_ticks < SDL_GetTicks()) gamestate=0;
    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
		if (event.key.keysym.sym == KEY_START)
			gamestate=3;
		else if (event.key.keysym.sym == KEY_QUIT)
			retval=1;
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

/* menu.c - menu */

#include "menu.h"

long mx=0, my=0;

extern int newjoy;

void initmenu()
{
     SDL_Surface *surf=NULL;
     surf=IMG_Load("img/ui/menu.png");
     tempsurf = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);
     surf=IMG_Load("img/ui/checked.png");
     checkbox = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);
     surf=IMG_Load("img/ui/cursor.png");
     SDL_SetColorKey(surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(surf->format,0xFF,0xFF,0xFF));
     mcurs = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);

	cboxrect.w=checkbox->w;
	cboxrect.h=checkbox->h;
	cboxrect.x=559;

	mcrect.w=mcurs->w;
	mcrect.h=mcurs->h;
	newjoy = 0;
}

void destroymenu()
{
	 SDL_FreeSurface(tempsurf);
	 SDL_FreeSurface(checkbox);
	 SDL_FreeSurface(mcurs);
}

void drawmenu()
{
	SDL_BlitSurface(tempsurf,NULL,screen,NULL);

	if (muson)
	{
		cboxrect.y=311;
		SDL_BlitSurface(checkbox,NULL,screen,&cboxrect);
	}
	if (sfxon)
	{
		cboxrect.y=370;
		SDL_BlitSurface(checkbox,NULL,screen,&cboxrect);
	}

    mcrect.x=mx;
    mcrect.y=my;
    SDL_BlitSurface(mcurs,NULL,screen,&mcrect);

    /* Make sure everything is displayed on screen */
    SDL_Flip (screen);
    /* Don't run too fast */
    SDL_Delay (1);
}

int handlemenu()
{
    SDL_Event event;
    int retval=0;

    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
            if (event.key.keysym.sym == KEY_QUIT)
                retval=1;
            else
                gamestate=3;
            break;
        case SDL_MOUSEBUTTONUP:
		if (event.button.x>98 && event.button.x<299)
		{
			if (event.button.y>137 && event.button.y<206)
		             gamestate=3;
			if (event.button.y>314 && event.button.y<383)
				gamestate=2;
			if (event.button.y>486 && event.button.y<555)
				retval=1;
		}
		if (event.button.x>550 && event.button.x<692)
		{
			if (event.button.y>312 && event.button.y<331)
			{
				if (muson==0)
				{
					muson=1;
					Mix_PlayMusic(music,-1);
				}else{
					muson=0;
					Mix_HaltMusic();
				}
			}
			if (event.button.y>369 && event.button.y<388)
				sfxon=(sfxon==0 ? 1 : 0);
			if (event.button.y>410 && event.button.y<468)
			{
		             gamestate=8;
			}
		}
             break;
        case SDL_MOUSEMOTION:
             mx=event.motion.x;
             my=event.motion.y;
             break;
        case SDL_QUIT:
            retval = 1;
            break;
        default:
            break;
        }
    }
    
    if (joysticks>0)
    {
       if (SDL_JoystickGetButton(joy,0)) {
         if (newjoy) gamestate=3;
       } else if (SDL_JoystickGetButton(joy,1)) {
         if (newjoy) gamestate=2;
       } else {
         newjoy=1;
       }
    }
    return retval;
}

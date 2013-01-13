/* menu.c - menu */

#include "winlose.h"

extern int newjoy;

Uint32 wl_debounce;

void initgameover()
{
	if (muson==1) {
		music=Mix_LoadMUS("audio/lose.mod");
		Mix_PlayMusic(music,-1);
	}

     SDL_Surface *surf=NULL;
     surf=IMG_Load("img/ui/gameover.png");
     tempsurf = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);
     newjoy = 0;

	wl_debounce = SDL_GetTicks() + 1000;
}

void destroygameover()
{
	 SDL_FreeSurface(tempsurf);
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music=NULL;
}

void drawgameover()
{    
     long tempscore,i;
     SDL_Rect scorerect, tempscorerect;
	scorerect.h=32;
	scorerect.w=32;
	tempscorerect.h=32;
	tempscorerect.w=32;
	tempscorerect.y=0;

	SDL_BlitSurface(tempsurf,NULL,screen,NULL);
	
    scorerect.y=375;
	i=450;
	tempscore=score;
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

int handlegameover()
{
    SDL_Event event;
    int retval=0;

    if (joysticks>0)
    {
       if (SDL_JoystickGetButton(joy,0)) {
         if (newjoy) {
           if (score>tscore[2]) gamestate=7; else gamestate=0;
         }
       } else {
         newjoy=1;
       }
    }

    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
             if ((event.key.keysym.sym==KEY_B1 || event.key.keysym.sym==KEY_QUIT || event.key.keysym.sym==KEY_START) && wl_debounce < SDL_GetTicks()){
               if (score>tscore[2]) gamestate=7; else
             		gamestate=0;}
            break;
        case SDL_MOUSEBUTTONUP:
               if (score>tscore[2]) gamestate=7; else
             		gamestate=0;
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

void initvictory()
{
	if (muson==1) {
		music=Mix_LoadMUS("audio/win.mod");
		Mix_PlayMusic(music,-1);
	}
     SDL_Surface *surf=NULL;
     surf=IMG_Load("img/ui/victory.png");
     tempsurf = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);
     newjoy=0;

	wl_debounce = SDL_GetTicks() + 1000;
}

void destroyvictory()
{
	 SDL_FreeSurface(tempsurf);
	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music=NULL;
}

void drawvictory()
{
    long tempscore,i;
    SDL_Rect scorerect, tempscorerect;
	scorerect.h=32;
	scorerect.w=32;
	tempscorerect.h=32;
	tempscorerect.w=32;
	tempscorerect.y=0;

	SDL_BlitSurface(tempsurf,NULL,screen,NULL);
    /* Make sure everything is displayed on screen */
    
    scorerect.y=513;
	i=528;
	tempscore=score;
	while (tempscore > 0)
	{
          i=i-32;
          tempscorerect.x=32*(tempscore % 10);
          scorerect.x=i;
          SDL_BlitSurface(numbers,&tempscorerect,screen,&scorerect);
          tempscore=(long)(tempscore/10);
    }
    
    
    
    SDL_Flip (screen);
    /* Don't run too fast */
    SDL_Delay (1);
}

int handlevictory()
{
    SDL_Event event;
    int retval=0;

    if (joysticks>0)
    {
       if (SDL_JoystickGetButton(joy,0)) {
         if (newjoy) gamestate=4;
       } else {
         newjoy=1;
       }
    }
    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
        case SDL_KEYUP:
             if ((event.key.keysym.sym==KEY_B1 || event.key.keysym.sym==KEY_QUIT || event.key.keysym.sym==KEY_START) && wl_debounce < SDL_GetTicks())
             		gamestate=4;
            break;
        case SDL_MOUSEBUTTONUP:
		gamestate=4;
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

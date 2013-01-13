/* hspick.c - choose your hs color */

#include "hspick.h"

extern int tscore[3];
extern int red[3];
extern int green[3];
extern int blue[3];

int go[]={63,127,191,255};

long mxo=400, myo=300, mxochg=0, myochg = 0, mxo2,myo2;
SDL_Rect mcrect;

SDL_Rect coloblock;

Uint32 hspick_debounce,hspick_debounce2;

void inithspick()
{
     SDL_Surface *surf=NULL;
     surf=IMG_Load("img/ui/hspick.png");
     tempsurf = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);

     surf=IMG_Load("img/ui/cursor.png");
     SDL_SetColorKey(surf, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(surf->format,0xFF,0xFF,0xFF));
     mcursy = SDL_DisplayFormat(surf);
     SDL_FreeSurface(surf);

	mcrect.w=mcursy->w;
	mcrect.h=mcursy->h;

	mxo = 400; myo = 300;

	coloblock.w=32;
	coloblock.h=32;

	if (muson) {
	  music=Mix_LoadMUS("audio/pause.mod");
	  Mix_PlayMusic(music,-1);
        }

// Debounce from the game: give a second to let go of the keys.
	hspick_debounce = SDL_GetTicks() + 1000;
// Prevents cursor from running too fast at key press.
	hspick_debounce2 = SDL_GetTicks() + 1;
}

void destroyhspick()
{
     Mix_HaltMusic();
     Mix_FreeMusic(music);
	music = NULL;
	 SDL_FreeSurface(tempsurf);
	 SDL_FreeSurface(mcursy);
}

void drawhspick()
{
     int i, j;

	SDL_BlitSurface(tempsurf,NULL,screen,NULL);

     for (i=0; i<8; i++)
     for (j=0; j<8; j++)
     {
         coloblock.x=i*32+272;
         coloblock.y=j*32+172;
         SDL_FillRect(screen,&coloblock,
           SDL_MapRGB(screen->format,go[(int)((8*i+j)/16)],
           go[(int)((8*i+j)/4) % 4],go[(8*i+j) % 4]));
     }

    mcrect.x=mxo;
    mcrect.y=myo;
    SDL_BlitSurface(mcursy,NULL,screen,&mcrect);
    /* Make sure everything is displayed on screen */
    SDL_Flip (screen);
    /* Don't run too fast */
    SDL_Delay (1);
}

int handlehspick()
{
    SDL_Event event;
    int retval=0;

    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == KEY_UP)
				myochg = -1;
			if (event.key.keysym.sym == KEY_DOWN)
				myochg = 1;
			if (event.key.keysym.sym == KEY_LEFT)
				mxochg = -1;
			if (event.key.keysym.sym == KEY_RIGHT)
				mxochg = 1;
			break;
		case SDL_KEYUP:
			if (event.key.keysym.sym == KEY_UP ||
			    event.key.keysym.sym == KEY_DOWN)
				myochg = 0;
			if (event.key.keysym.sym == KEY_LEFT ||
			    event.key.keysym.sym == KEY_RIGHT)
				mxochg = 0;
			if (hspick_debounce < SDL_GetTicks()) {
			if (event.key.keysym.sym == KEY_QUIT) gamestate = 0;
			if (event.key.keysym.sym == KEY_B1 ||
			    event.key.keysym.sym == KEY_START) {
            mxo2=mxo-272;
            myo2=myo-172;
            mxo2/=32;
            myo2/=32;
            if (mxo2 >= 0 && mxo2 < 8 && myo2 >= 0 && myo2 < 8) {
            if (score>tscore[0])
            {
               tscore[2]=tscore[1];
               tscore[1]=tscore[0];
               tscore[0]=score;
               red[2]=red[1];
               red[1]=red[0];
               red[0]=go[(int)((8*mxo2+myo2)/16)];
               green[2]=green[1];
               green[1]=green[0];
               green[0]=go[(int)((8*mxo2+myo2)/4)%4];
               blue[2]=blue[1];
               blue[1]=blue[0];
               blue[0]=go[(8*mxo2+myo2)%4];
            }
            else if (score>tscore[1])
            {
               tscore[2]=tscore[1];
               tscore[1]=score;
               red[2]=red[1];
               red[1]=go[(int)((8*mxo2+myo2)/16)];
               green[2]=green[1];
               green[1]=go[(int)((8*mxo2+myo2)/4)%4];
               blue[2]=blue[1];
               blue[1]=go[(8*mxo2+myo2)%4];
            }
            else
            {
               tscore[2]=score;
               red[2]=go[(int)((8*mxo2+myo2)/16)];
               green[2]=go[(int)((8*mxo2+myo2)/4)%4];
               blue[2]=go[(8*mxo2+myo2)%4];
            }
		    gamestate=0;
           }}}
			break;
        case SDL_MOUSEBUTTONUP:
		mxo = event.button.x;
		myo = event.button.y;

            mxo2=mxo-272;
            myo2=myo-172;
            mxo2/=32;
            myo2/=32;
            if (mxo2 >= 0 && mxo2 < 8 && myo2 >= 0 && myo2 < 8) {
            if (score>tscore[0])
            {
               tscore[2]=tscore[1];
               tscore[1]=tscore[0];
               tscore[0]=score;
               red[2]=red[1];
               red[1]=red[0];
               red[0]=go[(int)((8*mxo2+myo2)/16)];
               green[2]=green[1];
               green[1]=green[0];
               green[0]=go[(int)((8*mxo2+myo2)/4)%4];
               blue[2]=blue[1];
               blue[1]=blue[0];
               blue[0]=go[(8*mxo2+myo2)%4];
            }
            else if (score>tscore[1])
            {
               tscore[2]=tscore[1];
               tscore[1]=score;
               red[2]=red[1];
               red[1]=go[(int)((8*mxo2+myo2)/16)];
               green[2]=green[1];
               green[1]=go[(int)((8*mxo2+myo2)/4)%4];
               blue[2]=blue[1];
               blue[1]=go[(8*mxo2+myo2)%4];
            }
            else
            {
               tscore[2]=score;
               red[2]=go[(int)((8*mxo2+myo2)/16)];
               green[2]=go[(int)((8*mxo2+myo2)/4)%4];
               blue[2]=go[(8*mxo2+myo2)%4];
            }
		    gamestate=0;
           }
             break;
        case SDL_MOUSEMOTION:
             mxo=event.motion.x;
             myo=event.motion.y;
             break;
        case SDL_QUIT:
            retval = 1;
            break;
        default:
            break;
        }
    }

    if (hspick_debounce2 < SDL_GetTicks()) {
        hspick_debounce2 = SDL_GetTicks() + 1;
        mxo += mxochg; if (mxo < 0) mxo = 0; if (mxo > 800) mxo = 800;
        myo += myochg; if (myo < 0) myo = 0; if (myo > 600) myo = 600;
    }

    return retval;
}

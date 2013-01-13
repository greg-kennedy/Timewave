/* game.c - gameplay */

#include "game.h"
#include "math.h"

#define SCROLLSPEED 4
#define EXPLOSPEED 4
//#define SHIPSPEED 1.5

/* num imgs to load */
#define MAXSHIPS 4
#define MAXBULS 5
#define MAXENS 255
#define MAXEXP 5

#define PI 3.14159265358

/* max num on screen */
#define SCREENBUL 500
#define SCREENE 32
#define ALLSPAWN 128
#define SCREENEX 16

SDL_Surface *strips[42]={NULL};
SDL_Surface *shipimgs[MAXSHIPS]={NULL};
SDL_Surface *bulimgs[MAXBULS]={NULL};
SDL_Surface *eimgs[MAXENS]={NULL};
SDL_Surface *expimgs[MAXEXP*5]={NULL};
SDL_Surface *bg=NULL, *pauseimg=NULL;
SDL_Surface *overlay[3]={NULL};
int stripslist[128]={0};
float strippos=-168;
int stripindex=0;
float bgpos=-600;
float overlaypos=-600;
long tickslastupdate=0;
int bossloop=0;

float sintbl[360];
float invuln=0;
float bosstimer=0;

int spawnpos=0;
int glevel=1;
int lives=3;

SDL_Rect striprect;
SDL_Rect bgrect;
SDL_Rect shiprect;
SDL_Rect bulletrect;
SDL_Rect indicrect;
SDL_Rect erect;
SDL_Rect scorerect;
SDL_Rect tempscorerect;
SDL_Rect pauserect;

float shipx;
float shipy;
int shipimg;
float cooldown=0.0f;
float urate,rate=1,oldrate=1;
float speedbar=0;
float qboss;
int gpause=0;

unsigned char keysdown[7]={0,0,0,0,0,0,0};
unsigned char joystate[7]={0,0,0,0,0,0,0};
struct bullet bullets[SCREENBUL];
struct enemy elist[SCREENE];
struct spawninfo slist[ALLSPAWN];
struct explosion explo[SCREENEX];

Mix_Chunk *up = NULL;
Mix_Chunk *down = NULL;
Mix_Chunk *pshot = NULL;
Mix_Chunk *eshot = NULL;
Mix_Chunk *pdie = NULL;
Mix_Chunk *edie = NULL;
Mix_Chunk *bdie = NULL;
Mix_Chunk *spshot = NULL;
Mix_Chunk *seshot = NULL;
Mix_Chunk *spdie = NULL;
Mix_Chunk *sedie = NULL;
Mix_Chunk *sbdie = NULL;
Mix_Chunk *fpshot = NULL;
Mix_Chunk *feshot = NULL;
Mix_Chunk *fpdie = NULL;
Mix_Chunk *fedie = NULL;
Mix_Chunk *fbdie = NULL;

char buffer[30];

long scores[256] =
{0,100,50,100,250,50,500,0,500,100,
 50,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
   0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,
     0,0,0,0,0,0,0,0,0,0,
      0,0,0,0,0,0,0,0,0,0,
       0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,
         0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,
 0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
  0,0,0,0,0,0,0,0,0,0,
   0,0,100000,100000,100000,10,10,10,0,10000,
    50000,30000,40000,20000,100000,100000};

void initgame(int level)
{
	int i;
	SDL_Surface *temp=NULL;
	FILE* fp;

	glevel=level;
	gpause=0;
	bosstimer=0;
	bossloop=0;

	if (muson==1) {
		sprintf(buffer,"audio/%d.mod",level);
		music=Mix_LoadMUS(buffer);
		Mix_PlayMusic(music,-1);
	}

	if (sfxon==1) {
        up = Mix_LoadWAV("audio/sup.wav");
        down = Mix_LoadWAV("audio/sdn.wav");
        pshot = Mix_LoadWAV("audio/bul2.wav");
        eshot = Mix_LoadWAV("audio/bul1.wav");
        pdie = Mix_LoadWAV("audio/ex2.wav");
        edie = Mix_LoadWAV("audio/ex1.wav");
        bdie = Mix_LoadWAV("audio/ex3.wav");
        spshot = Mix_LoadWAV("audio/sbul2.wav");
        seshot = Mix_LoadWAV("audio/sbul1.wav");
        spdie = Mix_LoadWAV("audio/sex2.wav");
        sedie = Mix_LoadWAV("audio/sex1.wav");
        sbdie = Mix_LoadWAV("audio/sex3.wav");
        fpshot = Mix_LoadWAV("audio/fbul2.wav");
        feshot = Mix_LoadWAV("audio/fbul1.wav");
        fpdie = Mix_LoadWAV("audio/fex2.wav");
        fedie = Mix_LoadWAV("audio/fex1.wav");
        fbdie = Mix_LoadWAV("audio/fex3.wav");
	}

    for (i=0;i<360;i++)
        sintbl[i]=sin(PI * (float)i / 180);

	for (i=0;i<3;i++)
	{
		sprintf(buffer,"img/gfx/top%d.png",i);
		temp=IMG_Load(buffer);
		if (temp!=NULL)
		{
			SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0xFF,0x00,0xFF));
			overlay[i]=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
		}else{
			overlay[i]=NULL;
		}
	}

	for (i=0;i<MAXSHIPS;i++)
	{
		sprintf(buffer,"img/gfx/ship%d.png",i);
		temp=IMG_Load(buffer);
		if (temp!=NULL)
		{
			SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0xFF,0x00,0xFF));
			shipimgs[i]=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
		}else{
			shipimgs[i]=NULL;
		}
	}

	for (i=0;i<MAXENS;i++)
	{
		sprintf(buffer,"img/gfx/en%d.png",i);
		temp=IMG_Load(buffer);
		if (temp!=NULL)
		{
			SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0xFF,0x00,0xFF));
			eimgs[i]=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
		}else{
			eimgs[i]=NULL;
		}
	}

	for (i=0;i<MAXEXP*5;i++)
	{
		sprintf(buffer,"img/gfx/exp%d.png",i);
		temp=IMG_Load(buffer);
		if (temp!=NULL)
		{
			SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0xFF,0x00,0xFF));
			expimgs[i]=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
		}else{
			expimgs[i]=NULL;
		}
	}

	for (i=0;i<MAXBULS;i++)
	{
		sprintf(buffer,"img/gfx/bul%d.png",i);
		temp=IMG_Load(buffer);
		if (temp!=NULL)
		{
			SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0xFF,0x00,0xFF));
			bulimgs[i]=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
		}else{
			bulimgs[i]=NULL;
		}
	}

    if (level<6) {
	for (i=0;i<42;i++)
	{
		sprintf(buffer,"img/strips/%d/%d.png",level,i);
		temp=IMG_Load(buffer);
		if (temp!=NULL)
		{
			SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0xFF,0x00,0xFF));
			strips[i]=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
		}else{
			strips[i]=NULL;
		}
	}
    }

	sprintf(buffer,"data/%d.map",level);
	fp=fopen(buffer,"rb");
	for(i=0;i<128;i++)
		fread(&stripslist[i],1,1,fp);
	fclose(fp);

	temp=IMG_Load("img/ui/paused.png");
	if (temp!=NULL)
	{
		SDL_SetColorKey(temp, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(temp->format,0x00,0x00,0x00));
		pauseimg=SDL_DisplayFormat(temp);
		SDL_FreeSurface(temp);
    }

	sprintf(buffer,"data/%d.ens",level);
	fp=fopen(buffer,"r");
	for(i=0;i<ALLSPAWN;i++)
        if (!feof(fp))
                fscanf(fp,"%d %f %f %d %f %f %d\n",&slist[i].when,&slist[i].x,&slist[i].y,&slist[i].type,&slist[i].arg1,&slist[i].arg2,&slist[i].life);
        else
            slist[i].when=999;
	fclose(fp);

	temp=IMG_Load("img/gfx/bg.png");
	if (temp!=NULL)
	{
			bg=SDL_DisplayFormat(temp);
			SDL_FreeSurface(temp);
	}
	striprect.w=800;
	striprect.x=0;
	striprect.h=128;
	bgrect.x=0;
	bgrect.w=800;
	bgrect.h=128;
	shiprect.w=32;
	shiprect.h=32;
	bulletrect.w=16;
	bulletrect.h=16;
	indicrect.w=0;
	indicrect.h=16;
	indicrect.x=16;
	indicrect.y=568;
	scorerect.h=32;
	scorerect.w=32;
	scorerect.y=32;
	tempscorerect.h=32;
	tempscorerect.w=32;
	tempscorerect.y=0;
	pauserect.x=258;
	pauserect.y=280;
	pauserect.w=284;
	pauserect.h=40;
	
	for(i=0;i<SCREENE;i++)
        elist[i].type=0;
	for(i=0;i<SCREENBUL;i++)
        bullets[i].type=0;
	for(i=0;i<SCREENEX;i++)
        explo[i].type=0;

    strippos=-168;
    stripindex=0;
    bgpos=-600;
    overlaypos=-600;
    invuln=0;
    spawnpos=0;
    lives=5;
	shipx=400;
	shipy=500;
	shipimg=1;
    cooldown=0.0f;
    rate=1;
    speedbar=0;

    for (i=0;i<7;i++)
        keysdown[i]=0;

	tickslastupdate=SDL_GetTicks();
}
 
void destroygame()
{
	int i;

	for (i=0;i<MAXSHIPS;i++)
		if (shipimgs[i]!=NULL)
           SDL_FreeSurface(shipimgs[i]);

	for (i=0;i<MAXEXP*5;i++)
		if (expimgs[i]!=NULL)
			SDL_FreeSurface(expimgs[i]);

	for (i=0;i<3;i++)
		if (overlay[i]!=NULL)
           SDL_FreeSurface(overlay[i]);

	for (i=0;i<MAXBULS;i++)
		if (bulimgs[i]!=NULL)
           SDL_FreeSurface(bulimgs[i]);

	for (i=0;i<MAXENS;i++)
		if (eimgs[i]!=NULL)
           SDL_FreeSurface(eimgs[i]);

    if (bg!=NULL) SDL_FreeSurface(bg);

    if (glevel<6) {
	for (i=0;i<42;i++)
		if (strips[i]!=NULL)
			SDL_FreeSurface(strips[i]); }

	Mix_HaltMusic();
	Mix_FreeMusic(music);
	music=NULL;
	Mix_HaltChannel(-1);
	Mix_FreeChunk(up);
	Mix_FreeChunk(down);
	Mix_FreeChunk(pshot);
	Mix_FreeChunk(pdie);
	Mix_FreeChunk(eshot);
	Mix_FreeChunk(bdie);
	Mix_FreeChunk(edie);
	Mix_FreeChunk(spshot);
	Mix_FreeChunk(spdie);
	Mix_FreeChunk(seshot);
	Mix_FreeChunk(sbdie);
	Mix_FreeChunk(sedie);
	Mix_FreeChunk(fpshot);
	Mix_FreeChunk(fpdie);
	Mix_FreeChunk(feshot);
	Mix_FreeChunk(fbdie);
	Mix_FreeChunk(fedie);
}

void spawn (int newstrip)
{
     while (slist[spawnpos].when<=newstrip)
     {
           createenemy(slist[spawnpos].x,slist[spawnpos].y,slist[spawnpos].type,slist[spawnpos].arg1,slist[spawnpos].arg2,slist[spawnpos].life);
           spawnpos++;
     }           
}

/* This function draws to the screen; replace this with your own code! */
void draw ()
{
	int i;
	long tempscore;

    if (gpause==1)
    {
       SDL_BlitSurface(pauseimg,NULL,screen,&pauserect);
    } else {
	bgpos += .5 * urate * SCROLLSPEED;
	strippos+=urate * SCROLLSPEED;
	overlaypos+=2*urate * SCROLLSPEED;
	if (overlaypos>=0) overlaypos=-600;
	if (strippos>=-40) {strippos=-168; spawn(stripindex++);}
	if (bgpos>=0) bgpos=-600;
	bgrect.y=(int)bgpos;
	SDL_BlitSurface(bg,NULL,screen,&bgrect);
	bgrect.y=(int)bgpos+600;
	SDL_BlitSurface(bg,NULL,screen,&bgrect);

	if (stripindex>121) stripindex=121;

	for(i=0;i<6;i++)
	{
		striprect.y=(int)strippos+128*i;
		SDL_BlitSurface(strips[stripslist[stripindex+6-i]],NULL,screen,&striprect);
	}

	for (i=0;i<SCREENBUL;i++)
	{
		if (bullets[i].type!=0)
		{
			bulletrect.x=(int)(bullets[i].x+.5)-4;
			bulletrect.y=(int)(bullets[i].y+.5)-4;
			SDL_BlitSurface(bulimgs[bullets[i].type-1],NULL,screen,&bulletrect);
		}
	}

	for (i=0;i<SCREENE;i++)
	{
		if (elist[i].type!=0)
		{
            erect.w=eimgs[elist[i].type-1]->w;
            erect.h=eimgs[elist[i].type-1]->h;
			erect.x=(int)(elist[i].x+.5)-(erect.w/2);
			erect.y=(int)(elist[i].y+.5)-(erect.h/2);
			SDL_BlitSurface(eimgs[elist[i].type-1],NULL,screen,&erect);
		}
	}

	shiprect.x=0;
	shiprect.y=0;
	SDL_BlitSurface(shipimgs[1],NULL,screen,&shiprect);

	shiprect.x=(int)shipx-32;
	shiprect.y=(int)shipy-32;
	SDL_BlitSurface(shipimgs[shipimg],NULL,screen,&shiprect);

	for (i=0;i<SCREENEX;i++)
	{
		if (explo[i].type!=0)
		{
			erect.w=expimgs[5*(explo[i].type-1)]->w;
			erect.h=expimgs[5*(explo[i].type-1)]->h;
			erect.x=(int)(explo[i].x+.5)-(erect.w/2);
			erect.y=(int)(explo[i].y+.5)-(erect.h/2);
			SDL_BlitSurface(expimgs[(explo[i].type-1)*5+((int)explo[i].img)/EXPLOSPEED],NULL,screen,&erect);
		}
	}

    indicrect.w=(int)speedbar;
    SDL_FillRect(screen, &indicrect, SDL_MapRGB(screen->format,0,0,255));

	bgrect.y=(int)overlaypos;
	SDL_BlitSurface(overlay[(int)rate],NULL,screen,&bgrect);
	bgrect.y=(int)overlaypos+600;
	SDL_BlitSurface(overlay[(int)rate],NULL,screen,&bgrect);
	
	i=768;
	tempscore=score;
	while (tempscore > 0 || i==768)
	{
          i=i-32;
          tempscorerect.x=32*(tempscore % 10);
          scorerect.x=i;
          SDL_BlitSurface(numbers,&tempscorerect,screen,&scorerect);
          tempscore=(long)(tempscore/10);
    }
    }
    
    tempscorerect.x=32*(lives-1);
    scorerect.x=64;
    SDL_BlitSurface(numbers,&tempscorerect,screen,&scorerect);

    SDL_Flip (screen);
    /* Don't run too fast */
    SDL_Delay (1);
}

int handlegame()
{
    Sint16 axis;
    Uint8 button;

    SDL_Event event;
    int retval=0;

    /* Check for events */
    while (SDL_PollEvent (&event))
    {
        switch (event.type)
        {
	case SDL_KEYDOWN:
         if (gpause==0) {
		if (event.key.keysym.sym == KEY_DOWN)
			keysdown[0]=1;
		if (event.key.keysym.sym == KEY_UP)
			keysdown[1]=1;
		if (event.key.keysym.sym == KEY_LEFT)
			keysdown[2]=1;
		if (event.key.keysym.sym == KEY_RIGHT)
			keysdown[3]=1;
		if (event.key.keysym.sym == KEY_B1)
			keysdown[4]=1;
		if (event.key.keysym.sym == KEY_B3)
			keysdown[5]=1;
		if (event.key.keysym.sym == KEY_B2)
			keysdown[6]=1;
/*		if (event.key.keysym.sym == KEY_9)
		   lives++;
		if (event.key.keysym.sym == KEY_0)
		   gamestate=6; */
         }
        break;
        case SDL_KEYUP:
		if (event.key.keysym.sym == KEY_DOWN)
			keysdown[0]=0;
		if (event.key.keysym.sym == KEY_UP)
			keysdown[1]=0;
		if (event.key.keysym.sym == KEY_LEFT)
			keysdown[2]=0;
		if (event.key.keysym.sym == KEY_RIGHT)
			keysdown[3]=0;
		if (event.key.keysym.sym == KEY_B1)
			keysdown[4]=0;
		if (event.key.keysym.sym == KEY_B3)
			keysdown[5]=0;
		if (event.key.keysym.sym == KEY_B2)
			keysdown[6]=0;
		if (event.key.keysym.sym == KEY_QUIT)
			gamestate=0;
/*		if (event.key.keysym.sym == '0')
			gamestate=6;  */
		if ( event.key.keysym.sym == KEY_PAUSE )
		{
             if (gpause == 0) {
            	if (muson==1) {
                 	Mix_HaltMusic();
                  	Mix_FreeMusic(music);
               		music=Mix_LoadMUS("audio/pause.mod");
               		Mix_PlayMusic(music,-1);
               	}
               gpause=1;
        } else {
            	if (muson==1) {
                 	Mix_HaltMusic();
                  	Mix_FreeMusic(music);
             		sprintf(buffer,"audio/%d.mod",glevel);
               		music=Mix_LoadMUS(buffer);
               		Mix_PlayMusic(music,-1);
               	}
               gpause=0;
               }
        }
            break;
        case SDL_QUIT:
            retval = 1;
            break;
        default:
            break;
        }
    }

    /* here we use the joystick to fake keypresses : ) */
    if (joysticks>0)
    {
       axis = SDL_JoystickGetAxis(joy, 0);
       if (axis < -10923) {
         joystate[2] = 1;
         joystate[3] = 0;
       } else if (axis > 10923) {
         joystate[2] = 0;
         joystate[3] = 1;
       } else {
         joystate[2] = 0;
         joystate[3] = 0;
       }

       axis = SDL_JoystickGetAxis(joy, 1);
       if (axis < -10923) {
         joystate[0] = 0;
         joystate[1] = 1;
       } else if (axis > 10923) {
         joystate[0] = 1;
         joystate[1] = 0;
       } else {
         joystate[0] = 0;
         joystate[1] = 0;
       }
       
       button = SDL_JoystickGetButton(joy, 0);
       if (button) joystate[6]=1; else joystate[6]=0;
       button = SDL_JoystickGetButton(joy, 1);
       if (button) joystate[4]=1; else joystate[4]=0;
       button = SDL_JoystickGetButton(joy, 2);
       if (button) joystate[5]=1; else joystate[5]=0;
    }

    return retval;

}

int updategame()
{
	int i,j;
	float tempticks;
	
	tempticks=(float)SDL_GetTicks();

	urate = rate * (tempticks-(float)tickslastupdate)/16.666667f;
	tickslastupdate=(long)tempticks;

    if (!gpause) {

	if (cooldown>0.0f) cooldown-=urate;
	if (keysdown[0]!=0 || joystate[0]!=0)
		shipy=shipy+(urate*SCROLLSPEED / rate);
	else if (keysdown[1]!=0 || joystate[1]!=0)
		shipy=shipy-(urate * SCROLLSPEED / rate);
	if (keysdown[2]!=0 || joystate[2]!=0)
	{
		shipx=shipx-(urate * SCROLLSPEED / rate);
		shipimg=0; }
  	else if (keysdown[3]!=0 || joystate[3]!=0)
   {
		shipx=shipx+(urate * SCROLLSPEED / rate);
		shipimg=2;
	} else shipimg=1;

	if (( joystate[4]!=0 || keysdown[4]!=0) && cooldown<=0.0f)
	{
		createbullet(shipx-32,shipy-16,1,90,7.5);
		createbullet(shipx+32,shipy-16,1,90,7.5);
		cooldown=8;
		if (sfxon) playnoise(4);
	}
	
	if (keysdown[5]!=0 || joystate[5]!=0)
	{
       rate=2;
    }else if ((keysdown[6]!=0 || joystate[6]!=0 ) && speedbar>0)
    {
       rate=0.5;
    }else {
          rate=1;
    }
 
    if (rate<oldrate && sfxon==1) playnoise(7);
    if (rate>oldrate && sfxon==1) playnoise(6);
    oldrate=rate;

    if (shipx<0) shipx=0;
    if (shipx>800) shipx=800;
    if (shipy<0) shipy=0;
    if (shipy>600) shipy=600;

    if (rate==2) speedbar+=urate*4;
    if (rate==0.5) speedbar-=urate*16;
    
    if (speedbar>768) speedbar=768;
    if (speedbar<0) speedbar=0;

	for (i=0;i<SCREENEX;i++)
	{
        if (explo[i].type!=0)
        {
           explo[i].img+=urate;
           explo[i].y+=(urate * SCROLLSPEED);
           if (explo[i].img>=5*EXPLOSPEED) {
	     if (explo[i].type==3) {
			gamestate=6;
      } else if (explo[i].type==1 && lives<=0) { gamestate=5;
		} else {explo[i].type=0;}
        }  }
    }

	for (i=0;i<SCREENBUL;i++)
	{
		if (bullets[i].type!=0)
		{
			bullets[i].x=bullets[i].x+(urate * (bullets[i].speed*sintbl[(bullets[i].theta+90) % 360]));
			bullets[i].y=bullets[i].y-(urate * (bullets[i].speed*sintbl[bullets[i].theta]));
			if (bullets[i].type==1)
			{
                for (j=0;j<SCREENE; j++)
                {
                    if (elist[j].type!=0)
                    {
                       if ((elist[j].x-(eimgs[elist[j].type-1]->w/2)<bullets[i].x) && (elist[j].x+(eimgs[elist[j].type-1]->w/2)>bullets[i].x) && (elist[j].y-(eimgs[elist[j].type-1]->h/2)<bullets[i].y) && (elist[j].y+(eimgs[elist[j].type-1]->h/2)>bullets[i].y))
                       {
                            bullets[i].type=0;
                            elist[j].life--;
                       }
                    }
                }
            }else if (invuln<=0) {
                  if ((shipx-32<bullets[i].x) && (shipx+32>bullets[i].x) && (shipy-32<bullets[i].y) && (shipy+32>bullets[i].y))
                  {
                     bullets[i].type=0;
                     invuln=100;
                     createexplosion(shipx,shipy,1);
                     lives--;
                  }
            }
            if (bullets[i].y<0 || bullets[i].y>600 || bullets[i].x<0 || bullets[i].x>800) bullets[i].type=0;
		}
	}

	for (i=0;i<SCREENE;i++)
	{
        if (elist[i].type!=0)
        {
                switch (elist[i].type) {
                case 1:                      // stationary turret
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>10)
                   {
                      createbullet(elist[i].x,elist[i].y+16,2,270,5.0);
                      if (sfxon) playnoise(5);
                      elist[i].arg1=0;
                   }
                   elist[i].y+=SCROLLSPEED*urate;
                   break;
                case 2:                     // shrapnel mine
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>5)
                   {
                      createbullet(elist[i].x,elist[i].y,3,elist[i].arg2,5.0);
                      if (sfxon) playnoise(5);
                      elist[i].arg2+=15;
                      if (elist[i].arg2>359) elist[i].arg2=0;
                      elist[i].arg1=0;
                   }
                   elist[i].y+=SCROLLSPEED*urate;
                     break;
                case 3:                      // single homer
                   elist[i].x+=SCROLLSPEED*2*urate*elist[i].arg2;
                   if (elist[i].arg3==0 && ((elist[i].x>elist[i].arg1 && elist[i].arg2>0) || (elist[i].x<elist[i].arg1 && elist[i].arg2<0)))
                   {
                      createenemy(elist[i].x,elist[i].y,246,0,270,1);
                      if (sfxon) playnoise(5);
                      elist[i].arg3=1;
                   }
                   elist[i].y+=SCROLLSPEED*urate;
                   break;
                case 4:                          // charger
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>5)
                   {
                      createbullet(elist[i].x,elist[i].y+16,2,265+(rand()%10),10.0);
                      if (sfxon) playnoise(5);
                      elist[i].arg1=0;
                   }
                   elist[i].y+=SCROLLSPEED*1.5*urate;
                   break;
                case 5:                          // depth charge
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>elist[i].arg2)
                   {
                      createbullet(elist[i].x,elist[i].y+16,5,0,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,45,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,90,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,135,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,180,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,225,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,270,10.0);
                      createbullet(elist[i].x,elist[i].y+16,5,315,10.0);
                      if (sfxon) playnoise(5);
                      createexplosion(elist[i].x,elist[i].y,4);
                      elist[i].type=0;
                      elist[i].arg1=0;
                   }
                   elist[i].y+=SCROLLSPEED*urate;
                   break;
                case 6:                          //side rocket guy
                   elist[i].y-=urate;
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>1)
                   {
                     if (elist[i].arg3==1)
                     {
                       elist[i].arg2-=1;
                       if (elist[i].arg2<=0) { elist[i].arg3=0; elist[i].arg2=0;}
                       } else {
                      createbullet(elist[i].x,elist[i].y-32,4,0,10.0);
                      createbullet(elist[i].x,elist[i].y-4,4,0,10.0);
                      createbullet(elist[i].x,elist[i].y+22,4,0,10.0);
                      createbullet(elist[i].x,elist[i].y-32,4,180,10.0);
                      createbullet(elist[i].x,elist[i].y-4,4,180,10.0);
                      createbullet(elist[i].x,elist[i].y+22,4,180,10.0);
                      if (sfxon) playnoise(5);
                      elist[i].arg2+=1;
                      if (elist[i].arg2>=20) elist[i].arg3=1; }
                      elist[i].arg1=0;
                   }
                   break;
                case 7:                          //brick!
                   elist[i].y+=SCROLLSPEED*urate;
                   break;
                case 8:                          //carrier: launches fighters
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>100)
                   {
                      createenemy(elist[i].x,elist[i].y,9,0,0,2);
                      if (sfxon) playnoise(5);
                      elist[i].arg1=0;
                   }
                   break;
                case 9:                         //carrier fighter!
                   elist[i].y+=SCROLLSPEED*1.5*urate;
                   if (elist[i].x<shipx) elist[i].x+=urate;
                   if (elist[i].x>shipx) elist[i].x-=urate;
                   elist[i].arg1+=urate;
                   if (elist[i].arg1>15)
                   {
                      createbullet(elist[i].x-8,elist[i].y,5,270,10.0);
                      createbullet(elist[i].x+8,elist[i].y,5,270,10.0);
                      if (sfxon) playnoise(5);
                      elist[i].arg1=0;
                   }
                   break;
                case 10:                         //uberbomb.
                   elist[i].y+=urate*SCROLLSPEED;
                   if (elist[i].y>300)
                   {
                     elist[i].type=0;
                     createexplosion(400,300,5);
                     speedbar=0;
                   }
                   break;
                case 242:
                case 243:
                case 244:             // space station!
                     elist[i].arg1+=urate;
                     if (elist[i].arg1>7)      //animate
                     {
                        elist[i].type+=1; if (elist[i].type==245) elist[i].type=242;
                        elist[i].arg1=0;
                     }
                     if (elist[i].y<128) elist[i].y+=urate*SCROLLSPEED; else {
                        // do all battle actions here.
                         elist[i].y=128;
                         if (elist[i].arg3>0)   // taking a shot
                         {
                            if (elist[i].arg3==1)  // machine gun!
                            {
                               elist[i].arg3=101;
                            }
                            if (elist[i].arg3==3)
                            {
                               bosstimer+=urate;
                               if (bosstimer>5)
                               {
                                     for (qboss=210;qboss<=330;qboss+=30)
                                       createenemy(elist[i].x,elist[i].y,246,0,qboss,1);
                                     if (sfxon) playnoise(5);
                                     bosstimer=0;
                                     elist[i].arg2+=1; if (elist[i].arg2>4) {elist[i].arg2=0; elist[i].arg3=0;}
                               }
                            }
                            if (elist[i].arg3==2)
                            {
                               createenemy(elist[i].x,elist[i].y,10,0,0,1);
                      if (sfxon) playnoise(5);
                               elist[i].arg3=0;
                               bosstimer=0;
                               elist[i].arg2=0;
                            }
                            if (elist[i].arg3==5)
                            {
                               createenemy(elist[i].x-50,elist[i].y,2,0,0,5);
                               createenemy(elist[i].x+50,elist[i].y,2,0,180,5);
                               createenemy(elist[i].x,elist[i].y,5,0,20,5);
                      if (sfxon) playnoise(5);
                               elist[i].arg3=0;
                               bosstimer=0;
                               elist[i].arg2=0;
                            }
                            if (elist[i].arg3==4)
                            {
                               createenemy(0,0,7,0,0,100);
                               createenemy(100,0,7,0,0,100);
                               createenemy(200,0,7,0,0,100);
                               createenemy(600,0,7,0,0,100);
                               createenemy(700,0,7,0,0,100);
                               createenemy(800,0,7,0,0,100);
                      if (sfxon) playnoise(5);
                               elist[i].arg3=0;
                               bosstimer=0;
                               elist[i].arg2=0;
                            }
                            if (elist[i].arg3>100)
                            {
                               bosstimer+=urate;
                               if (bosstimer>1)
                               {
                                  bosstimer=0;
                                  elist[i].arg3+=1;
                                  elist[i].arg2=180*atan2((-shipy+elist[i].y),(shipx-elist[i].x))/PI;
                                  if (elist[i].arg2<0) elist[i].arg2+=360;
                                  createbullet(elist[i].x,elist[i].y,5,elist[i].arg2,10);
                      if (sfxon) playnoise(5);
                                  elist[i].arg2=0;
                                  if (elist[i].arg3>200) {elist[i].arg3=0; }
                               }
                            }
                         } else {
                           elist[i].arg2+=urate;
                           if (elist[i].arg2>150) { elist[i].arg2=0; elist[i].arg3=bossloop; bossloop+=1; if (bossloop>5) bossloop=1; }
                         }
                     }
                     
                     break;
                case 245:
                case 246:
                case 247:              // homing rocket
                     if (elist[i].arg2<225) elist[i].type=245;
                     else if (elist[i].arg2>315) elist[i].type=247;
                     else elist[i].type=246;

                     elist[i].arg1+=urate;
                     if (elist[i].arg1 > 20)     // can't adjust angle at first
                     {
                         elist[i].arg3=(180*atan2((-shipy+elist[i].y),(shipx-elist[i].x))/PI);
                         if (elist[i].arg3<0) elist[i].arg3+=360;
                         elist[i].arg2+=(elist[i].arg2<elist[i].arg3?2*urate:2*(-urate));
                         if (elist[i].arg2>330) elist[i].arg2=330;
                         if (elist[i].arg2<210) elist[i].arg2=210;
                     }
                     elist[i].x+=10*urate*sintbl[(int)(elist[i].arg2+90) % 360];
                     elist[i].y-=10*urate*sintbl[(int)elist[i].arg2];
                     break;
                case 248:                // laser boss helper
                     elist[i].arg1+=urate;
                     if (elist[i].arg1>20)
                        elist[i].type=0;
                     else
                     {
                         elist[i].arg3+=urate;
                         if (elist[i].arg3>1)
                         {
                            elist[i].arg3=0;
                            createbullet(elist[i].x,elist[i].y,5,elist[i].arg2,10);
                      if (sfxon) playnoise(5);
                         }
                     }
                     break;
                case 249:                // shotgun boss
                     elist[i].arg1+=urate;
                     if (elist[i].arg1>100)
                     {
                        if (sfxon) playnoise(5);
                        if (elist[i].arg3==0) elist[i].arg1=240;
                        if (elist[i].arg3==1 || elist[i].arg3==7) elist[i].arg1=255;
                        if (elist[i].arg3==2 || elist[i].arg3==6) elist[i].arg1=270;
                        if (elist[i].arg3==3 || elist[i].arg3==5) elist[i].arg1=285;
                        if (elist[i].arg3==4) elist[i].arg1=300;
                        elist[i].arg3=elist[i].arg3+1;
                        if (elist[i].arg3==8) elist[i].arg3=0;
                        for (elist[i].arg2=210;elist[i].arg2<elist[i].arg1-5;elist[i].arg2++)
                           createbullet(elist[i].x,elist[i].y,2,elist[i].arg2,7.5);
                        for (elist[i].arg2=elist[i].arg1+5;elist[i].arg2<=330;elist[i].arg2++)
                           createbullet(elist[i].x,elist[i].y,2,elist[i].arg2,7.5);
                        elist[i].arg1=0;
                     }
                     break;
                case 250:               //shrapnel boss
                     elist[i].arg1+=urate;
                     elist[i].x+=elist[i].arg2;
                     if (elist[i].x<128) elist[i].arg2=1;
                     if (elist[i].x>672) elist[i].arg2=-1;
                     if (elist[i].arg1>150)
                     {
                        createenemy(elist[i].x,elist[i].y,2,0,0,5);
                      if (sfxon) playnoise(5);
                        elist[i].arg1=0;
                     }
                     break;
                case 251:               //laser boss
                     elist[i].arg1+=urate;
                     if (elist[i].arg1>20)
                     {
                        elist[i].arg2=180*atan2((-shipy+elist[i].y),(shipx-elist[i].x))/PI;
                        if (elist[i].arg2<0) elist[i].arg2+=360;
                        createenemy(elist[i].x,elist[i].y,248,0,elist[i].arg2,1);
                        elist[i].arg1=0;
                     }
                     break;
                case 252:               //rocket boss
                     elist[i].arg1+=urate;
                     elist[i].x+=elist[i].arg2;
                     if (elist[i].x<300) elist[i].arg2=1;
                     if (elist[i].x>500) elist[i].arg2=-1;
                     if (elist[i].arg1>50)
                     {
                        for (elist[i].arg3=210;elist[i].arg3<=330;elist[i].arg3+=30)
                            createenemy(elist[i].x,elist[i].y,246,0,elist[i].arg3,1);
                      if (sfxon) playnoise(5);
                        elist[i].arg1=0;
                     }
                     break;
                case 253:               //dcboss
                     elist[i].arg1+=urate;
                     elist[i].x+=elist[i].arg2;
                     if (elist[i].x<128) elist[i].arg2=1;
                     if (elist[i].x>672) elist[i].arg2=-1;
                     if (elist[i].arg1>100)
                     {
                        createenemy(elist[i].x,elist[i].y,5,0,(shipy-elist[i].y)/SCROLLSPEED,5);
                      if (sfxon) playnoise(5);
                        elist[i].arg1=0;
                     }
                     break;
                default:
                   break;
                }
                if (elist[i].type!=0){
                         if (invuln==0)
                                     if ((elist[i].x-(eimgs[elist[i].type-1]->w/2)<shipx+32) && (elist[i].x+(eimgs[elist[i].type-1]->w/2)>shipx-32) && (elist[i].y-(eimgs[elist[i].type-1]->h/2)<shipy+32) && (elist[i].y+(eimgs[elist[i].type-1]->h/2)>shipy-32) )
                                     {
                                                    elist[i].life--;
                                                    lives--;
                                                    invuln=100;
                                                    createexplosion(shipx,shipy,1);
                                     }
                         if (elist[i].life<=0) {score+=scores[elist[i].type]; createexplosion(elist[i].x,elist[i].y,2);
                           if (elist[i].type>248 || (elist[i].type > 241 && elist[i].type < 245)) {
// Just for fun, make one of these big blue bombs go off.
			if (elist[i].type > 241 && elist[i].type < 245)
                     createexplosion(elist[i].x,elist[i].y,5);
                              createexplosion(elist[i].x-32,elist[i].y-32,1);
                              createexplosion(elist[i].x+32,elist[i].y-32,3);
                              createexplosion(elist[i].x-32,elist[i].y+32,3);
                              createexplosion(elist[i].x+32,elist[i].y+32,1);
                           }
                         elist[i].type=0;}
                         if (elist[i].y<0 || elist[i].y>600 || elist[i].x<0 || elist[i].x>800) elist[i].type=0;
                }
		}
	}

    if (invuln>0) {invuln-=urate; shipimg=3;}
    if (invuln<0) invuln=0;

}
	return 0;
}

int createbullet(float x, float y, int type, int theta, float speed)
{
	int i;
	for(i=0;i<SCREENBUL;i++)
		if (bullets[i].type==0)
		{
			bullets[i].type=type;
			bullets[i].x=x;
			bullets[i].y=y;
			bullets[i].theta=theta;
			bullets[i].speed=speed;
			break;
		}

	return (i==SCREENBUL);

}

int createexplosion(float x, float y, int type)
{
	int i;
	for(i=0;i<SCREENEX;i++)
		if (explo[i].type==0)
		{
			explo[i].type=type;
			explo[i].x=x;
			explo[i].y=y;
			explo[i].img=0;
			break;
		}

    if (sfxon) {
      if (type==1 || type==3 ) playnoise(type);
      else playnoise(2);
    }

	return (i==SCREENEX);
}

int createenemy(float x, float y, int type, float arg1, float arg2,int life)
{
    if (type>248)
    {
        Mix_HaltMusic();
        Mix_FreeMusic(music);
        glevel=0;
		music=Mix_LoadMUS("audio/boss.mod");
		Mix_PlayMusic(music,-1);
    }        


	int i;
	for(i=0;i<SCREENE;i++)
		if (elist[i].type==0)
		{
			elist[i].type=type;
			elist[i].x=x;
			elist[i].y=y;
			elist[i].arg1=arg1;
			elist[i].arg2=arg2;
            elist[i].arg3=0;
            elist[i].life=life;
			break;
		}
	return (i==SCREENE);
}

void playnoise(int sample)
{
     if (sample==6) Mix_PlayChannel(-1,up,0);
     else if (sample==7) Mix_PlayChannel(-1,down,0);
     else if (rate==1)
     {
         if (sample==1) Mix_PlayChannel(-1,pdie,0);
         if (sample==2) Mix_PlayChannel(-1,edie,0);
         if (sample==3) Mix_PlayChannel(-1,bdie,0);
         if (sample==4) Mix_PlayChannel(-1,pshot,0);
         if (sample==5) Mix_PlayChannel(-1,eshot,0);
     }
     else if (rate==.5)
     {
         if (sample==1) Mix_PlayChannel(-1,spdie,0);
         if (sample==2) Mix_PlayChannel(-1,sedie,0);
         if (sample==3) Mix_PlayChannel(-1,sbdie,0);
         if (sample==4) Mix_PlayChannel(-1,spshot,0);
         if (sample==5) Mix_PlayChannel(-1,seshot,0);
     }
     else if (rate==2)
     {
         if (sample==1) Mix_PlayChannel(-1,fpdie,0);
         if (sample==2) Mix_PlayChannel(-1,fedie,0);
         if (sample==3) Mix_PlayChannel(-1,fbdie,0);
         if (sample==4) Mix_PlayChannel(-1,fpshot,0);
         if (sample==5) Mix_PlayChannel(-1,feshot,0);
     }
}

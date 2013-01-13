#include "common.h"
#include "menu.h"
#include "title.h"
#include "game.h"
#include "main.h"
#include "hs.h"
#include "winlose.h"
#include "hspick.h"
#include "keyconf.h"

//#include <windows.h>

/*
 PLEASE NOTE: the program will require SDL.dll which is located in
              dev-c++'s dll directory. You have to copy it to you
			  program's home directory or the path.
 */

int joysticks;
SDL_Joystick *joy;

SDLKey keys[10];

int arcade_mode;

int main (int argc, char *argv[])
{
    int done, i;
    int level=1;
    SDL_Surface *tempsurf;
	int fullscreen;
    FILE *hifile;

int create_config = 0;

	// ----------------------------------------------------------------------------
	// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__    
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    }
    CFRelease(resourcesURL);
	
    chdir(path);
    //std::cout << "Current Path: " << path << std::endl;
#endif
	// ----------------------------------------------------------------------------
	
    /* Initialize SDL */
    if (SDL_Init (SDL_INIT_TIMER | SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
    {
        printf ("Couldn't initialize SDL: %s\n", SDL_GetError ());
        exit (1);
    }
    atexit (SDL_Quit);

int redo_opts = 0;
    hifile=fopen("config.ini","r");
    if (hifile!=NULL)
    {
      if (fscanf(hifile,"FULLSCREEN=%d\nMUSIC=%d\nSFX=%d\nARCADE=%d\nKEY_UP=%d\nKEY_DOWN=%d\nKEY_LEFT=%d\nKEY_RIGHT=%d\nKEY_FIRE1=%d\nKEY_FIRE2=%d\nKEY_FIRE3=%d\nKEY_START=%d\nKEY_PAUSE=%d\nKEY_QUIT=%d\n",&fullscreen,&muson,&sfxon,&arcade_mode,&keys[0],&keys[1],&keys[2],&keys[3],&keys[4],&keys[5],&keys[6],&keys[7],&keys[8],&keys[9]) != 14) redo_opts = 1;
      for (i=0; i<3; i++)
        if (fscanf(hifile,"SCORE=%d,%d,%d,%d\n",&tscore[i],&red[i], &green[i], &blue[i]) != 4) redo_opts = 1;
      fclose(hifile);
    } else redo_opts = 1;

    if (redo_opts)
    {
	fullscreen = 1;
	muson = 1;
	sfxon = 1;
	arcade_mode = 0;
	keys[0] = SDLK_UP;
	keys[1] = SDLK_DOWN;
	keys[2] = SDLK_LEFT;
	keys[3] = SDLK_RIGHT;
	keys[4] = SDLK_SPACE;
	keys[5] = SDLK_LSHIFT;
	keys[6] = SDLK_LCTRL;
	keys[7] = SDLK_SPACE;
	keys[8] = SDLK_p;
	keys[9] = SDLK_ESCAPE;

      tscore[0]=3;
      tscore[1]=2;
      tscore[2]=1;
    }

    /* Set 640x480 16-bits video mode */
    if (argc > 1)
    {
      if (strcmp(argv[1], "-w") == 0)
      {
	fullscreen=0;
      } else if (strcmp(argv[1], "-f") == 0) {
	fullscreen=1;
      } else if (strcmp(argv[1], "-c") == 0) {
	create_config = 1;
      }
    }

if (!create_config) {
#ifdef __APPLE__
    screen = SDL_SetVideoMode(800, 600, 16, SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen?SDL_FULLSCREEN:0));
#else
    screen = SDL_SetVideoMode(800, 600, (fullscreen?16:0), SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen?SDL_FULLSCREEN:0));
#endif

    if (screen == NULL)
    {
        printf ("Couldn't set 800x600x16 video mode: %s\n",SDL_GetError ());
        exit (2);
    }
    SDL_WM_SetCaption ("Timewave", NULL);
    SDL_ShowCursor( SDL_DISABLE );

    int audio_rate = 44100; //MIX_DEFAULT_FREQUENCY;
    Uint16 audio_format = MIX_DEFAULT_FORMAT;
    int audio_channels = MIX_DEFAULT_CHANNELS;
    int audio_buffers = 4096;

    if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
      printf("Unable to open audio!\n");
      muson=0;
      sfxon=0;
      //exit(1);
    }

    Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);

    joysticks = SDL_NumJoysticks();
    if (joysticks > 0) joy = SDL_JoystickOpen(0);
    if (joy==NULL) joysticks=0;

    tempsurf = IMG_Load("img/ui/nums.png");
	if (tempsurf!=NULL)
	{
		SDL_SetColorKey(tempsurf, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(tempsurf->format,0xFF,0xFF,0xFF));
		numbers=SDL_DisplayFormat(tempsurf);
		SDL_FreeSurface(tempsurf);
	}else{
		numbers=NULL;
	}

    done = 0;
    while (!done)
    {
          if(gamestate==0)
          {
              inittitle();
               while (!done && gamestate==0)
               {
                    done=handletitle();
                    drawtitle();
               }
               destroytitle();
          }else if (gamestate==1)
          {
              initmenu();
               while (!done && gamestate==1)
               {
                done=handlemenu();
		if (gamestate==3)
		{
			Mix_HaltMusic();
			Mix_FreeMusic(music);
		}
                drawmenu();
               }
               destroymenu();
          }else if (gamestate==2){
              iniths();
               while (!done && gamestate==2)
               {
                done=handlehs();
                drawhs();
               }
               destroyhs();
          }else if (gamestate==3){
                level=1;
                score=0;
                gamestate=4;
          }else if (gamestate==4){
              initgame(level);
               while (!done && gamestate==4)
               {
                done=handlegame();
		        updategame();
                draw();
               }
               destroygame();
          }else if (gamestate==5){
                initgameover();
               while (!done && gamestate==5)
               {
                done=handlegameover();
                drawgameover();
               }
               destroygameover();
          }else if (gamestate==6){
                initvictory();
               while (!done && gamestate==6)
               {
                done=handlevictory();
                drawvictory();
               }
               destroyvictory();
               level++;
               if (level>6) gamestate=5;
          }else if (gamestate==7){
              inithspick();
               while (!done && gamestate==7)
               {
                done=handlehspick();
                drawhspick();
               }
               destroyhspick();
          }else if (gamestate==8){
              initkeyconf();
               while (!done && gamestate==8)
               {
                done=handlekeyconf();
                drawkeyconf();
               }
               destroykeyconf();
          }else{
                done=1;           /* should not reach this point. */
          }
    }

    SDL_FreeSurface (numbers);

    if (music != NULL)
    {
         Mix_HaltMusic();
         Mix_FreeMusic(music);
    }
    Mix_CloseAudio();

}

    hifile=fopen("config.ini","w");
    if (hifile!=NULL)
    {
      fprintf(hifile,"FULLSCREEN=%d\nMUSIC=%d\nSFX=%d\nARCADE=%d\nKEY_UP=%d\nKEY_DOWN=%d\nKEY_LEFT=%d\nKEY_RIGHT=%d\nKEY_FIRE1=%d\nKEY_FIRE2=%d\nKEY_FIRE3=%d\nKEY_START=%d\nKEY_PAUSE=%d\nKEY_QUIT=%d\n",fullscreen, muson,sfxon,arcade_mode,keys[0],keys[1],keys[2],keys[3],keys[4],keys[5],keys[6],keys[7],keys[8],keys[9]);
      for (i=0; i<3; i++)
        fprintf(hifile,"SCORE=%d,%d,%d,%d\n",tscore[i],red[i], green[i], blue[i]);
      fclose(hifile);
    }

    if (SDL_JoystickOpened(0)) SDL_JoystickClose(joy);

    SDL_ShowCursor( SDL_ENABLE );
    SDL_Quit();

    return 0;
}

// common includes
#include "common.h"

// various UI screens
#include "menu.h"
#include "title.h"
#include "game.h"
#include "hs.h"
#include "winlose.h"
#include "hspick.h"
#include "keyconf.h"

SDL_Surface *screen = NULL;
SDL_Surface *numbers = NULL;
int gamestate=0;
int muson=1, sfxon=1;
long score=0;

SDL_Surface *tempsurf = NULL;
Mix_Music *music = NULL;   /* SDL_Mixer stuff here */

int tscore[3]={0};
int red[3]={0};
int green[3]={0};
int blue[3]={0};

extern SDL_Joystick *joy;
extern int joysticks;

int joysticks;
SDL_Joystick *joy;

SDLKey keys[10];

int arcade_mode;

// configuration struct
struct cfg_struct *cfg;

// helper to-string function
static const char* to_string(unsigned int i)
{
	static char buffer[11];
	snprintf(buffer,11,"%u",i);
	return buffer;
}

int main (int argc, char *argv[])
{
	printf("Timewave v1.3 - Greg Kennedy 2016\n");

	// ----------------------------------------------------------------------------
	// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__    
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8 *)path, PATH_MAX))
    {
        // error!
    	fprintf(stderr,"Failed call to CFURLGetFileSystemRepresentation.\n");
    	return 1;
    }
    CFRelease(resourcesURL);
	
	printf("\tChanged path to %s.\n",path);
    chdir(path);
#endif
	// ----------------------------------------------------------------------------

	// Read config files.
	/* Initialize config struct */
	cfg = cfg_init();

	/* Specifying some defaults */
	cfg_set(cfg,"FULLSCREEN", "1");
	cfg_set(cfg,"MUSIC", "1");
	cfg_set(cfg,"SFX", "1");
	cfg_set(cfg,"ARCADE", "0");
	cfg_set(cfg,"KEY_UP", to_string(SDLK_UP));
	cfg_set(cfg,"KEY_DOWN", to_string(SDLK_DOWN));
	cfg_set(cfg,"KEY_LEFT", to_string(SDLK_LEFT));
	cfg_set(cfg,"KEY_RIGHT", to_string(SDLK_RIGHT));
	cfg_set(cfg,"KEY_FIRE1", to_string(SDLK_SPACE));
	cfg_set(cfg,"KEY_FIRE2", to_string(SDLK_LSHIFT));
	cfg_set(cfg,"KEY_FIRE3", to_string(SDLK_LCTRL));
	cfg_set(cfg,"KEY_START", to_string(SDLK_SPACE));
	cfg_set(cfg,"KEY_PAUSE", to_string(SDLK_p));
	cfg_set(cfg,"KEY_QUIT", to_string(SDLK_ESCAPE));
	cfg_set(cfg,"AUDIO_RATE", to_string(MIX_DEFAULT_FREQUENCY));
	cfg_set(cfg,"AUDIO_FORMAT", to_string(MIX_DEFAULT_FORMAT));
	cfg_set(cfg,"AUDIO_CHANNELS", to_string(MIX_DEFAULT_CHANNELS));
	cfg_set(cfg,"AUDIO_BUFFERS", "4096");
	cfg_set(cfg,"SCORE_1","3,0,0,0");
	cfg_set(cfg,"SCORE_2","2,0,0,0");
	cfg_set(cfg,"SCORE_3","1,0,0,0");

	// Now overwrite defaults by reading from file
	printf("\tReading config information from config.ini.\n");
	cfg_load(cfg,"config.ini");

	// And read command-line params
    for (int i=1; i<argc; i++)
    {
		if (strcmp(argv[i], "-w") == 0)
		{
			cfg_set(cfg,"FULLSCREEN","0");
		} else if (strcmp(argv[1], "-f") == 0) {
			cfg_set(cfg,"FULLSCREEN","1");
    	}
	}

	// Time to initialize SDL.
    int done;
    int level=1;
    SDL_Surface *tempsurf;

    /* Initialize SDL */
    if (SDL_Init (SDL_INIT_TIMER | SDL_INIT_VIDEO) < 0)
    {
        fprintf (stderr,"Couldn't initialize SDL (timer, video): %s\n", SDL_GetError ());
        exit (1);
    }
    atexit (SDL_Quit);

	/* Optional components */
	int audio_works = (SDL_InitSubSystem(SDL_INIT_AUDIO) == 0);
	if (!audio_works)
        fprintf (stderr,"Couldn't initialize SDL Audio subsystem: %s\n", SDL_GetError ());

	int joystick_works = (SDL_InitSubSystem(SDL_INIT_JOYSTICK) == 0);
	if (!joystick_works)
        fprintf (stderr,"Couldn't initialize SDL Joystick subsystem: %s\n", SDL_GetError ());

    /* Set 800x600 16-bits video mode */
	int fullscreen = atoi(cfg_get(cfg,"FULLSCREEN"));
#ifdef __APPLE__
    screen = SDL_SetVideoMode(800, 600, 16, SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen?SDL_FULLSCREEN:0));
#else
    screen = SDL_SetVideoMode(800, 600, (fullscreen?16:0), SDL_HWSURFACE | SDL_DOUBLEBUF | (fullscreen?SDL_FULLSCREEN:0));
#endif

    if (screen == NULL)
    {
        fprintf (stderr,"Couldn't set 800x600x16 video mode: %s\n",SDL_GetError ());
        exit (2);
    }
    SDL_WM_SetCaption ("Timewave", NULL);
    SDL_ShowCursor( SDL_DISABLE );

    int audio_rate = cfg_get(cfg,"AUDIO_RATE")
    Uint16 audio_format = cfg_get(cfg,"AUDIO_FORMAT")
    int audio_channels = cfg_get(cfg,"AUDIO_CHANNELS");
    int audio_buffers = cfg_get(cfg,"AUDIO_BUFFERS");

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

	// Now overwrite defaults by reading from file
	cfg_save(cfg,"config.ini");

    if (SDL_JoystickOpened(0)) SDL_JoystickClose(joy);

    SDL_ShowCursor( SDL_ENABLE );
    SDL_Quit();

    return 0;
}

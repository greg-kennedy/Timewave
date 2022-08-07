/* ************************************************************************ */
// Apple bundle stuff
#ifdef __APPLE__
	#include "CoreFoundation/CoreFoundation.h"
#endif

// config file parsing
#include "cfg_parse/cfg_parse.h"

// common includes and GLOBALS
#include "common.h"

// various UI screens
#include "meta_ui.h"
// Gameplay sub-block
#include "meta_play.h"

/* ************************************************************************ */
// helper int-to-string function
static const char* to_string(int i)
{
	static char buffer[12];
	snprintf(buffer, 12, "%d", i);
	return buffer;
}

/* ************************************************************************ */
// Main entry point
int main (int argc, char* argv[])
{
	int ret = EXIT_SUCCESS;

	printf("Timewave v1.3 - Greg Kennedy 2016\n");

	// ----------------------------------------------------------------------------
	// This makes relative paths work in C++ in Xcode by changing directory to the Resources folder inside the .app bundle
#ifdef __APPLE__
	CFBundleRef mainBundle = CFBundleGetMainBundle();
	CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
	char path[PATH_MAX];
	if (! CFURLGetFileSystemRepresentation(resourcesURL, TRUE, (UInt8*)path, PATH_MAX))
	{
		// error!
		fprintf(stderr, "Failed call to CFURLGetFileSystemRepresentation.\n");
		return EXIT_FAILURE;
	}
	CFRelease(resourcesURL);

	chdir(path);
	printf("\tChanged path to %s.\n", path);
#endif

	// ----------------------------------------------------------------------------
	// Read config files and unpack values.

	/* Initialize config struct */
	struct cfg_struct* cfg =
		cfg_init();

	/* Specifying some defaults */
	cfg_set(cfg, "FULLSCREEN", "1");
	cfg_set(cfg, "ARCADE", "0");
	cfg_set(cfg, "MUSIC", "1");
	cfg_set(cfg, "SFX", "1");

	cfg_set(cfg, "KEY_UP", to_string(SDLK_UP));
	cfg_set(cfg, "KEY_DOWN", to_string(SDLK_DOWN));
	cfg_set(cfg, "KEY_LEFT", to_string(SDLK_LEFT));
	cfg_set(cfg, "KEY_RIGHT", to_string(SDLK_RIGHT));
	cfg_set(cfg, "KEY_FIRE", to_string(SDLK_SPACE));
	cfg_set(cfg, "KEY_SPEED_DOWN", to_string(SDLK_LSHIFT));
	cfg_set(cfg, "KEY_SPEED_UP", to_string(SDLK_LCTRL));
	cfg_set(cfg, "KEY_START", to_string(SDLK_RETURN));
	cfg_set(cfg, "KEY_PAUSE", to_string(SDLK_p));
	cfg_set(cfg, "KEY_QUIT", to_string(SDLK_ESCAPE));

	cfg_set(cfg, "AUDIO_RATE", to_string(MIX_DEFAULT_FREQUENCY));
	cfg_set(cfg, "AUDIO_FORMAT", to_string(MIX_DEFAULT_FORMAT));
	cfg_set(cfg, "AUDIO_CHANNELS", to_string(MIX_DEFAULT_CHANNELS));
	cfg_set(cfg, "AUDIO_BUFFERS", MIX_DEFAULT_BUFFERS);

	cfg_set(cfg, "SCORE_1", "3,0,0,0");
	cfg_set(cfg, "SCORE_2", "2,0,0,0");
	cfg_set(cfg, "SCORE_3", "1,0,0,0");

	// Now override defaults by reading from file
	printf("Reading config information from config.ini.\n");
	cfg_load(cfg, "config.ini");

	// other config files on command line
	for (int i = 1; i < argc; i ++)
	{
		printf("\tReading additional config information from %s.\n", argv[i]);
		cfg_load(cfg, argv[i]);
	}

	// ----------------------------------------------------------------------------
	// Create "common" object to share data with subsystems
	struct env_t env;

	// ----------------------------------------------------------------------------
	// Initialize SDL - mandatory subsystems
	if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO) != 0)
	{
		fprintf(stderr, "SDL_Init: ERROR: Couldn't initialize SDL (timer, video): %s\n", SDL_GetError());
		return EXIT_FAILURE;
	}

	/* Image Loader */
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		fprintf(stderr, "IMG_Init: ERROR: Failed to IMG_INIT_PNG: %s\n", IMG_GetError());
		SDL_Quit();
		return EXIT_FAILURE;
	}

	// ----------------------------------------------------------------------------
	/* Optional components */
	/* AUDIO MIXER */
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	{
		fprintf(stderr, "Couldn't initialize SDL Audio subsystem: %s\n", SDL_GetError());
		env.sfx_works = 0;
		env.mus_works = 0;
	} else {
		// try opening the audio mixer api
		int audio_rate = atoi(cfg_get(cfg, "AUDIO_RATE"));
		Uint16 audio_format = atoi(cfg_get(cfg, "AUDIO_FORMAT"));
		int audio_channels = atoi(cfg_get(cfg, "AUDIO_CHANNELS"));
		const int audio_buffers = atoi(cfg_get(cfg, "AUDIO_BUFFERS"));

		if(Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers) != 0)
		{
			fprintf(stderr, "Mix_OpenAudio: ERROR: failed open with params: %s\n", Mix_GetError());

			SDL_QuitSubSystem(SDL_INIT_AUDIO);

			env.sfx_works = 0;
			env.mus_works = 0;
		} else {
			// that worked, log some info, looks like we get SFX at a minimum
			const int times_opened = Mix_QuerySpec(&audio_rate, &audio_format, &audio_channels);
			printf("Opened audio mixer - Times: %d, Rate: %d, Format: %hu, Channels: %d\n", times_opened, audio_rate, audio_format, audio_channels);

			env.sfx_works = 1;

			// now try to get modfile loading
			if (Mix_Init(MIX_INIT_MOD) != MIX_INIT_MOD)
			{
				// failed to open MIKMOD library - no music
				fprintf(stderr, "Mix_Init: ERROR: failed to MIX_INIT_MOD: %s\n", Mix_GetError());
				env.mus_works = 0;
				Mix_Quit();
			} else {
				env.mus_works = 1;
			}
		}
	}

	/* JOYSTICK SUPPORT */
	SDL_Joystick* joy = NULL;
	if (SDL_InitSubSystem(SDL_INIT_JOYSTICK) != 0)
	{
		fprintf(stderr, "Couldn't initialize SDL Joystick subsystem: %s\n", SDL_GetError());
	} else {
		if (SDL_NumJoysticks() > 0)
		{
			joy = SDL_JoystickOpen(0);
			if (joy == NULL)
			{
				fprintf(stderr, "Failed to open joystick ID 0: %s\n", SDL_GetError());
				SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
			} else {
			    printf("Opened Joystick 0\n");
			    printf("\tName: %s\n", SDL_JoystickName(0));
			    printf("\tNumber of Axes: %d\n", SDL_JoystickNumAxes(joy));
				printf("\tNumber of Buttons: %d\n", SDL_JoystickNumButtons(joy));
  			}
		} else {
			printf("\tInitialized joystick subsystem, but no sticks found.\n");
			SDL_QuitSubSystem(SDL_INIT_JOYSTICK);
		}
	}

	// ----------------------------------------------------------------------------
	// TIME TO SET UP THE GAME!
	// ----------------------------------------------------------------------------

	/* Set 800x600 16-bits video mode */
	env.screen = SDL_SetVideoMode(800, 600, 0, SDL_HWSURFACE | SDL_DOUBLEBUF | (atoi(cfg_get(cfg, "FULLSCREEN")) ? SDL_FULLSCREEN : 0));
	if (env.screen == NULL)
	{
		fprintf(stderr, "Couldn't set 800x600 video mode: %s\n", SDL_GetError());
		goto error;
	}
	SDL_WM_SetCaption ("Timewave", NULL);
	SDL_ShowCursor( SDL_DISABLE );

	/* Set a custom filter for the various SDL events */
	if (joy != NULL)
		SDL_JoystickEventState(SDL_ENABLE);

	SDL_EventState(SDL_JOYBALLMOTION, SDL_IGNORE);
	SDL_EventState(SDL_JOYHATMOTION, SDL_IGNORE);
	SDL_EventState(SDL_VIDEORESIZE, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	/* SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE); */

	/* disable key repeat */
	SDL_EnableKeyRepeat(0, 0);

	// ----------------------------------------------------------------------------
	/* load the shared resources */
	if (init_common() != 0)
	{
		fprintf(stderr, "Failed to init shared resources\n");
		goto error;
	}

	env.cursor = load_image("img/ui/cursor.png", 1);
	if (env.cursor == NULL)
	{
		free_common();
		goto error;
	}

	// ----------------------------------------------------------------------------
	/* read sfx and music switches from cfg */
	env.sfx_on = atoi(cfg_get(cfg, "SFX"));
	env.mus_on = atoi(cfg_get(cfg, "MUSIC"));

	/* Read key config from cfg */
	env.KEY_UP = atoi(cfg_get(cfg, "KEY_UP"));
	env.KEY_DOWN = atoi(cfg_get(cfg, "KEY_DOWN"));
	env.KEY_LEFT = atoi(cfg_get(cfg, "KEY_LEFT"));
	env.KEY_RIGHT = atoi(cfg_get(cfg, "KEY_RIGHT"));
	env.KEY_FIRE = atoi(cfg_get(cfg, "KEY_FIRE"));
	env.KEY_SPEED_DOWN = atoi(cfg_get(cfg, "KEY_SPEED_DOWN"));
	env.KEY_SPEED_UP = atoi(cfg_get(cfg, "KEY_SPEED_UP"));
	env.KEY_START = atoi(cfg_get(cfg, "KEY_START"));
	env.KEY_PAUSE = atoi(cfg_get(cfg, "KEY_PAUSE"));
	env.KEY_QUIT = atoi(cfg_get(cfg, "KEY_QUIT"));

	/* get high scores from cfg */
	sscanf(cfg_get(cfg, "SCORE_1"), "%10d,%3d,%3d,%3d", &(env.hs_score[0]), &(env.hs_red[0]), &(env.hs_green[0]), &(env.hs_blue[0]));
	sscanf(cfg_get(cfg, "SCORE_2"), "%10d,%3d,%3d,%3d", &(env.hs_score[1]), &(env.hs_red[1]), &(env.hs_green[1]), &(env.hs_blue[1]));
	sscanf(cfg_get(cfg, "SCORE_3"), "%10d,%3d,%3d,%3d", &(env.hs_score[2]), &(env.hs_red[2]), &(env.hs_green[2]), &(env.hs_blue[2]));

	// ----------------------------------------------------------------------------
	/* MAIN STATE LOOP */
	// ----------------------------------------------------------------------------
	int arcade_mode = atoi(cfg_get(cfg, "ARCADE"));
	int gamestate = STATE_UI;
	do
	{
		switch(gamestate)
		{
			case STATE_UI:
				gamestate = state_ui(&env, arcade_mode);
				break;

			case STATE_PLAY:
				gamestate = state_play(&env);
				break;

			case STATE_ERROR:
				fprintf(stderr, "Terminating game due to unrecoverable error.\n");
				gamestate = STATE_QUIT;
				break;

			default:
				fprintf(stderr, "ERROR: Reached unknown gamestate %d\n", gamestate);
				gamestate = STATE_ERROR;
		}
	} while (gamestate != STATE_QUIT);

	// ----------------------------------------------------------------------------
	// Shut it all down!
	// ----------------------------------------------------------------------------
	SDL_FreeSurface(env.cursor);
	free_common();

	// Now save our current settings back to the cfg struct
	// SFX and Music switches
	cfg_set(cfg, "SFX", to_string(env.sfx_on));
	cfg_set(cfg, "MUSIC", to_string(env.mus_on));

	// key config
	cfg_set(cfg, "KEY_UP", to_string(env.KEY_UP));
	cfg_set(cfg, "KEY_DOWN", to_string(env.KEY_DOWN));
	cfg_set(cfg, "KEY_LEFT", to_string(env.KEY_LEFT));
	cfg_set(cfg, "KEY_RIGHT", to_string(env.KEY_RIGHT));
	cfg_set(cfg, "KEY_FIRE", to_string(env.KEY_FIRE));
	cfg_set(cfg, "KEY_SPEED_DOWN", to_string(env.KEY_SPEED_DOWN));
	cfg_set(cfg, "KEY_SPEED_UP", to_string(env.KEY_SPEED_UP));
	cfg_set(cfg, "KEY_START", to_string(env.KEY_START));
	cfg_set(cfg, "KEY_PAUSE", to_string(env.KEY_PAUSE));
	cfg_set(cfg, "KEY_QUIT", to_string(env.KEY_QUIT));

	// high score table
	char buffer[50] = {0};
	sprintf(buffer, "%d,%d,%d,%d", env.hs_score[0], env.hs_red[0], env.hs_green[0], env.hs_blue[0]);
	cfg_set(cfg, "SCORE_1", buffer);
	sprintf(buffer, "%d,%d,%d,%d", env.hs_score[1], env.hs_red[1], env.hs_green[1], env.hs_blue[1]);
	cfg_set(cfg, "SCORE_2", buffer);
	sprintf(buffer, "%d,%d,%d,%d", env.hs_score[2], env.hs_red[2], env.hs_green[2], env.hs_blue[2]);
	cfg_set(cfg, "SCORE_3", buffer);

	// save config to disk
	cfg_save(cfg, "config.ini");

	// close up everything and exit
	goto cleanup;

error:
	ret = EXIT_FAILURE;
cleanup:
	if (SDL_JoystickOpened(0)) SDL_JoystickClose(joy);
	if (env.mus_works) Mix_Quit();
	if (env.sfx_works) Mix_CloseAudio();
	IMG_Quit();
	SDL_Quit();

	return ret;
}

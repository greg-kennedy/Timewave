/* common.h - globals */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <SDL/SDL.h>
#ifdef __APPLE__
  #include "SDL_mixer/SDL_mixer.h"
  #include "SDL_image/SDL_image.h"

  #include "CoreFoundation/CoreFoundation.h"
#else
  #include <SDL/SDL_mixer.h>
  #include <SDL/SDL_image.h>
#endif

#include "cfg_parse/cfg_parse.h"

#define KEY_UP keys[0]
#define KEY_DOWN keys[1]
#define KEY_LEFT keys[2]
#define KEY_RIGHT keys[3]
#define KEY_B1 keys[4]
#define KEY_B2 keys[5]
#define KEY_B3 keys[6]
#define KEY_START keys[7]
#define KEY_PAUSE keys[8]
#define KEY_QUIT keys[9]

#include "common.h"

/* ************************************************************************************************************* */
// File-level globals
static SDL_Surface* numbers;

/* ************************************************************************************************************* */
int init_common()
{
	numbers = load_image("img/ui/nums.png", 1);
	if (numbers == NULL) return 1;

	return 0;
}

void free_common()
{
	SDL_FreeSurface(numbers);
}

/* ************************************************************************************************************* */
/* load an image from a file */
SDL_Surface* load_image(const char* path, const int alpha)
{
	SDL_Surface* img = IMG_Load(path);
	if (img == NULL)
	{
		fprintf(stderr, "load_image: ERROR: Failed to load image '%s': %s\n", path, IMG_GetError());
		return NULL;
	}

	if (alpha)
	{
		// try to set the alpha chanel to Magic Pink
		if (SDL_SetColorKey(img, SDL_SRCCOLORKEY | SDL_RLEACCEL, SDL_MapRGB(img->format, 0xFF, 0x00, 0xFF)) != 0)
			fprintf(stderr, "load_image_alpha: Warning: Failed to set color key for image '%s': %s\n", path, SDL_GetError());
	}

	// convert loaded surface to our display format
	SDL_Surface* conv = SDL_DisplayFormat(img);

	if (conv == NULL)
	{
		fprintf(stderr, "load_image: Warning: Failed to convert image '%s': %s\n", path, SDL_GetError());

		// the original may still be usable though
		return img;
	}

	// don't need img any more
	SDL_FreeSurface(img);

	return conv;
}

/* load and (maybe) play some music */
Mix_Music* load_music(const char* path, const int autoplay)
{
	Mix_Music* music = Mix_LoadMUS(path);

	if (music != NULL) {
		/* loaded OK... if mus_on, try playing it too */
		if (autoplay) {
			if (Mix_PlayMusic(music, -1) != 0) {
				/* failure when playing */
				fprintf(stderr, "load_music(%s): Warning: Mix_PlayMusic failed: %s\n", path, Mix_GetError());
			}
		}
	} else {
		/* totally failed to load */
		fprintf(stderr, "load_music(%s): Warning: Mix_LoadMUS returned NULL: %s\n", path, Mix_GetError());
	}

	return music;
}

Mix_Chunk* load_sample(const char* path)
{
	Mix_Chunk* sample = Mix_LoadWAV(path);

	if (!sample)
		fprintf(stderr, "load_sample(%s): Warning: Mix_LoadWAV returned NULL: %s\n", path, Mix_GetError());

	return sample;
}

/* write a number using the numbers format to screen
	this is funny because it's right-to-left but whatever */
void blit_number(SDL_Surface* dest, const unsigned int x, const unsigned int y, int number)
{
	SDL_Rect dstrect;
	dstrect.x = x;
	dstrect.y = y;

	SDL_Rect srcrect;
	srcrect.h = 32;
	srcrect.w = 24;
	srcrect.y = 0;

	// write one digit at a time to screen, right to left
	do {
		srcrect.x = 24 * (number % 10);
		SDL_BlitSurface(numbers, &srcrect, dest, &dstrect);
		dstrect.x -= 24;
		number /= 10;
	} while (number > 0);
}

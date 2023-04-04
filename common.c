#include "common.h"

#include <zlib.h>
#include <SDL/SDL_rwops.h>

/* ************************************************************************************************************* */
// RWops wrappers for gzip (zlib)
static void sdl_rwops_gzperror(const gzFile file, const char * caller)
{
	int errnum;
	const char * message = gzerror(file, &errnum);
	fprintf(stderr, "zlib error in %s: %s\n", caller, message);

	if (errnum == Z_ERRNO) perror("\tZ_ERRNO detail");
}

static int SDLCALL sdl_rwops_gzread(SDL_RWops * context, void * ptr, int size, int maxnum)
{
//	z_size_t ret = gzfread(ptr, size, maxnum, context->hidden.unknown.data1);
	int ret = gzread(context->hidden.unknown.data1, ptr, size * maxnum) / size;

	if (ret == 0 && !gzeof(context->hidden.unknown.data1))
		sdl_rwops_gzperror(context->hidden.unknown.data1, "gzread");

	return ret;
}

static int SDLCALL sdl_rwops_gzseek(SDL_RWops * context, int offset, int whence)
{
	// workaround for SEEK_END
	if (whence == 2) {
		// seek way past the end
		//  this ought to return INT_MAX
		int ret = sdl_rwops_gzseek(context, INT_MAX, 0);

		if (ret == -1) {
			// hmm some kind of weird error there
			return -1;
		} else if (ret != INT_MAX)
			fputs("Warning: gzseek(INT_MAX) did not return INT_MAX\n", stderr);

		// do a dummy read of 1 byte - this will always fail but should set us to eof
		char dummy;
		ret = sdl_rwops_gzread(context, &dummy, 1, 1);

		if (ret != 0) {
			// this was supposed to fail...
			fputs("Warning: gzread past eof did not return 0\n", stderr);
		}

		// great now we can seek relative to *current* position
		whence = 1;
	}

	z_off_t ret = gzseek(context->hidden.unknown.data1, offset, whence);

	if (ret == -1)
		sdl_rwops_gzperror(context->hidden.unknown.data1, "gzseek");

	return ret;
}

static int SDLCALL sdl_rwops_gzclose(SDL_RWops * context)
{
//	int ret = gzclose_r(context->hidden.unknown.data1);
	int ret = gzclose(context->hidden.unknown.data1);

	switch (ret) {
	case Z_STREAM_ERROR:
		fputs("gzclose_r: Z_STREAM_ERROR: file is not valid\n", stderr);
		break;

	case Z_MEM_ERROR:
		fputs("gzclose_r: Z_MEM_ERROR: out of memory\n", stderr);
		break;

	case Z_BUF_ERROR:
		fputs("gzclose_r: Z_BUF_ERROR: last read ended in the middle of a gzip stream\n", stderr);
		break;

	case Z_ERRNO:
		perror("gzclose_r: Z_ERRNO: file operation error");
	}

	SDL_FreeRW(context);
	return ret;
}

SDL_RWops * sdl_gzopen(const char * file)
{
	SDL_RWops * rwops = SDL_AllocRW();

	if (!rwops)
		return NULL;

	rwops->hidden.unknown.data1 = gzopen(file, "r");

	if (rwops->hidden.unknown.data1 == NULL) {
		fputs("gzopen returned NULL: ", stderr);
		perror(file);
		SDL_FreeRW(rwops);
		return NULL;
	}

	rwops->seek  = sdl_rwops_gzseek;
	rwops->read  = sdl_rwops_gzread;
	rwops->write = NULL;
	rwops->close = sdl_rwops_gzclose;
	return rwops;
}

/* ************************************************************************************************************* */
// File-level globals
static SDL_Surface * numbers;

/* ************************************************************************************************************* */
int init_common(void)
{
	numbers = load_image("img/ui/nums.png", 3);

	if (numbers == NULL) return 1;

	return 0;
}

void free_common(void)
{
	SDL_FreeSurface(numbers);
}

/* ************************************************************************************************************* */
/* load an image from a file */
SDL_Surface * load_image(const char * path, const int alpha)
{
	SDL_Surface * img = IMG_Load(path);

	if (img == NULL) {
		fprintf(stderr, "load_image: ERROR: Failed to load image '%s': %s\n", path, IMG_GetError());
		return NULL;
	}

	if (alpha) {
		// try to set the alpha chanel to Magic Pink
		int flags = SDL_SRCCOLORKEY;

		// RLE acceleration enabled by setting bit 2 - this is good for bg and overlay,
		//  not so much for bullets and enemy images
		if (alpha & 0x02) flags |= SDL_RLEACCEL;

		if (SDL_SetColorKey(img, flags, SDL_MapRGB(img->format, 0xFF, 0x00, 0xFF)) != 0)
			fprintf(stderr, "load_image_alpha: Warning: Failed to set color key for image '%s': %s\n", path, SDL_GetError());
	}

	// convert loaded surface to our display format
	SDL_Surface * conv = SDL_DisplayFormat(img);

	if (conv == NULL) {
		fprintf(stderr, "load_image: Warning: Failed to convert image '%s': %s\n", path, SDL_GetError());
		// the original may still be usable though
		return img;
	}

	// don't need img any more
	SDL_FreeSurface(img);
	return conv;
}

/* load and (maybe) play some music */
Mix_Music * load_music(const char * path, const int autoplay)
{
	SDL_RWops * gz = sdl_gzopen(path);

	if (! gz) {
		fprintf(stderr, "load_music(%s): Warning: sdl_gzopen failed\n", path);
		return NULL;
	}

	Mix_Music * music = Mix_LoadMUS_RW(gz);
	SDL_RWclose(gz);

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
		fprintf(stderr, "load_music(%s): Warning: Mix_LoadMUS_RW returned NULL: %s\n", path, Mix_GetError());
	}

	return music;
}

Mix_Chunk * load_sample(const char * path)
{
	Mix_Chunk * sample = Mix_LoadWAV_RW(sdl_gzopen(path), 1);

	if (!sample)
		fprintf(stderr, "load_sample(%s): Warning: Mix_LoadWAV returned NULL: %s\n", path, Mix_GetError());

	return sample;
}

/* write a number using the numbers format to screen
	this is funny because it's right-to-left but whatever */
void blit_number(SDL_Surface * dest, const unsigned int x, const unsigned int y, int number)
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

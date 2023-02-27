/* hspick.c - choose your hs color */

#include "hspick.h"

#include "common.h"

#include "meta_play.h"

int state_hspick(struct env_t * env, Mix_Music * music_pause, const int score)
{
	/* Load the resources for this screen */
	SDL_Surface * bg = load_image("img/ui/hspick.png", 0);

	if (bg == NULL) return STATE_ERROR;

	/* Reuses the pause music */
	Mix_PlayMusic(music_pause, -1);
	int go[] = {63, 127, 191, 255};
	SDL_Rect coloblock;
	coloblock.w = 32;
	coloblock.h = 32;
	// key state
	unsigned char keysdown[4] = {0};
	// mouse state
	int mx, my;
	SDL_GetMouseState(&mx, &my);
	// Debounce from the game: give a second to let go of the keys.
	const Uint32 hspick_debounce = SDL_GetTicks() + 1000;
	// set to redraw the screen
	int dirty = 1;
	/* Event loop */
	int state = STATE_HSPICK;

	do {
		unsigned char color_pushed = 0;
		/* read events */
		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_VIDEOEXPOSE:
				dirty = 1;
				break;

			case SDL_KEYDOWN:
				if (event.key.keysym.sym == env->KEY_UP)
					keysdown[0] = 1;
				else if (event.key.keysym.sym == env->KEY_DOWN)
					keysdown[1] = 1;
				else if (event.key.keysym.sym == env->KEY_LEFT)
					keysdown[2] = 1;
				else if (event.key.keysym.sym == env->KEY_RIGHT)
					keysdown[3] = 1;

				break;

			case SDL_KEYUP:
				if (event.key.keysym.sym == env->KEY_UP)
					keysdown[0] = 0;
				else if (event.key.keysym.sym == env->KEY_DOWN)
					keysdown[1] = 0;
				else if (event.key.keysym.sym == env->KEY_LEFT)
					keysdown[2] = 0;
				else if (event.key.keysym.sym == env->KEY_RIGHT)
					keysdown[3] = 0;
				else if ((event.key.keysym.sym == env->KEY_FIRE ||
						event.key.keysym.sym == env->KEY_START) && hspick_debounce < SDL_GetTicks())
					color_pushed = 1;
				else if (event.key.keysym.sym == env->KEY_QUIT  && hspick_debounce < SDL_GetTicks()) state = STATE_UI;

				break;

			case SDL_JOYAXISMOTION:
				if (event.jaxis.axis == 0) { // left-right movement
					if (event.jaxis.value < -4095) {
						keysdown[2] = 1;
						keysdown[3] = 0;
					} else if (event.jaxis.value > 4095) {
						keysdown[2] = 0;
						keysdown[3] = 1;
					} else {
						keysdown[2] = 0;
						keysdown[3] = 0;
					}
				} else if (event.jaxis.axis == 1) { // up-down axis
					if (event.jaxis.value < -4095) {
						keysdown[0] = 1;
						keysdown[1] = 0;
					} else if (event.jaxis.value > 4095) {
						keysdown[0] = 0;
						keysdown[1] = 1;
					} else {
						keysdown[0] = 0;
						keysdown[1] = 0;
					}
				}

				break;

			case SDL_JOYBUTTONUP:
				if (event.jbutton.button == env->buttons[0] && hspick_debounce < SDL_GetTicks())
					color_pushed = 1;

				break;

			case SDL_MOUSEBUTTONUP:
				mx = event.button.x;
				my = event.button.y;
				color_pushed = 1;
				dirty = 1;
				break;

			case SDL_MOUSEMOTION:
				mx = event.motion.x;
				my = event.motion.y;
				dirty = 1;
				break;

			case SDL_QUIT:
				state = STATE_QUIT;
			}
		}

		/* apply keypresses */
		if (keysdown[0] && my > 0) {
			my--;
			dirty = 1;
		}

		if (keysdown[1] && my < 600) {
			my++;
			dirty = 1;
		}

		if (keysdown[2] && mx > 0) {
			mx--;
			dirty = 1;
		}

		if (keysdown[3] && mx < 800) {
			mx++;
			dirty = 1;
		}

		/* apply lockin */
		if (color_pushed) {
			int mxo2 = mx - 272;
			int myo2 = my - 172;
			mxo2 /= 32;
			myo2 /= 32;

			if (mxo2 >= 0 && mxo2 < 8 && myo2 >= 0 && myo2 < 8) {
				if (score > env->hs_score[0]) {
					env->hs_score[2] = env->hs_score[1];
					env->hs_score[1] = env->hs_score[0];
					env->hs_score[0] = score;
					env->hs_red[2] = env->hs_red[1];
					env->hs_red[1] = env->hs_red[0];
					env->hs_red[0] = go[(int)((8 * mxo2 + myo2) / 16)];
					env->hs_green[2] = env->hs_green[1];
					env->hs_green[1] = env->hs_green[0];
					env->hs_green[0] = go[(int)((8 * mxo2 + myo2) / 4) % 4];
					env->hs_blue[2] = env->hs_blue[1];
					env->hs_blue[1] = env->hs_blue[0];
					env->hs_blue[0] = go[(8 * mxo2 + myo2) % 4];
				} else if (score > env->hs_score[1]) {
					env->hs_score[2] = env->hs_score[1];
					env->hs_score[1] = score;
					env->hs_red[2] = env->hs_red[1];
					env->hs_red[1] = go[(int)((8 * mxo2 + myo2) / 16)];
					env->hs_green[2] = env->hs_green[1];
					env->hs_green[1] = go[(int)((8 * mxo2 + myo2) / 4) % 4];
					env->hs_blue[2] = env->hs_blue[1];
					env->hs_blue[1] = go[(8 * mxo2 + myo2) % 4];
				} else {
					env->hs_score[2] = score;
					env->hs_red[2] = go[(int)((8 * mxo2 + myo2) / 16)];
					env->hs_green[2] = go[(int)((8 * mxo2 + myo2) / 4) % 4];
					env->hs_blue[2] = go[(8 * mxo2 + myo2) % 4];
				}

				state = STATE_UI;
			}
		}

		/* draw screen */
		if (dirty) {
			int i, j;
			SDL_BlitSurface(bg, NULL, env->screen, NULL);

			for (i = 0; i < 8; i++)
				for (j = 0; j < 8; j++) {
					coloblock.x = i * 32 + 272;
					coloblock.y = j * 32 + 172;
					SDL_FillRect(env->screen, &coloblock,
						SDL_MapRGB(env->screen->format, go[(int)((8 * i + j) / 16)],
							go[(int)((8 * i + j) / 4) % 4], go[(8 * i + j) % 4]));
				}

			SDL_Rect mcrect;
			mcrect.x = mx;
			mcrect.y = my;
			SDL_BlitSurface(env->cursor, NULL, env->screen, &mcrect);
			SDL_Flip(env->screen);
			dirty = 0;
		}

		SDL_Delay(1);
	} while (state == STATE_HSPICK);

	Mix_HaltMusic();
	SDL_FreeSurface(bg);
	return state;
}


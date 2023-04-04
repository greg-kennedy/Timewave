/* meta_ui.c - UI loop */
#include "meta_ui.h"

#include "common.h"

#include "title.h"
#include "hs.h"
#include "keyconf.h"
#include "menu.h"

/* *************************************************** */
/* SUB STATE HANDLER FOR TITLE/MENU/ETC UI */
int state_ui(struct env_t * env, const int arcade_mode)
{
	Mix_Music * music = NULL;

	// play the music for all UI items
	if (env->mus_works)
		music = load_music("audio/title.xm.gz", env->mus_on);

	int substate = STATE_TITLE;

	do {
		switch (substate) {
		case STATE_TITLE:
			substate = state_title(env, arcade_mode);
			break;

		case STATE_HS:
			substate = state_hs(env, arcade_mode);
			break;

		case STATE_MENU:
			substate = state_menu(env, music);
			break;

		case STATE_KEYCONF:
			substate = state_keyconf(env);
		}
	} while (substate == STATE_TITLE ||
		substate == STATE_HS ||
		substate == STATE_MENU ||
		substate == STATE_KEYCONF);

	// leaving this scene shuts off the music
	Mix_FreeMusic(music);
	return substate;
}

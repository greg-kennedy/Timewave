#include "meta_play.h"

#include "common.h"

#include "game.h"
#include "winlose.h"
#include "hspick.h"

int state_play(struct env_t* env)
{
	// preload game resources
	init_game(env);

	// PAUSE music
	Mix_Music* music_pause = NULL;
	if (env->mus_works && env->mus_on)
		music_pause = load_music("audio/pause.mod", 0);

	// reset the score and level
	int score = 0;
	int level = 1;

	int substate = STATE_GAME;
	do
	{
		switch(substate)
		{
			case STATE_GAME:
				substate = state_game(env, music_pause, level, &score);
				break;

			case STATE_VICTORY:
				substate = state_victory(env, score);
				break;

			case STATE_GAME_NEXT:
				level ++;
				if (level < 6)
					substate = STATE_GAME;
				else
					substate = STATE_GAMEOVER;
				break;

			case STATE_GAMEOVER:
				substate = state_gameover(env, score);
				break;

			case STATE_PLAY_END:
				if (score > env->hs_score[2])
					substate = STATE_HSPICK;
				else
					substate = STATE_UI;
				break;

			case STATE_HSPICK:
				substate = state_hspick(env, music_pause, score);
		}
	} while (substate == STATE_GAME ||
			substate == STATE_VICTORY ||
			substate == STATE_GAME_NEXT ||
			substate == STATE_GAMEOVER ||
			substate == STATE_PLAY_END ||
			substate == STATE_HSPICK);

	Mix_FreeMusic(music_pause);
	
	free_game();

	return substate;
}

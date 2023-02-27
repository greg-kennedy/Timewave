/* meta_play.h - Game loop */
#ifndef META_PLAY_H_
#define META_PLAY_H_

/* *************************************************** */
// Local state numbers
#define STATE_GAME 10
#define STATE_VICTORY 11
#define STATE_GAME_NEXT 12
#define STATE_GAMEOVER 13
#define STATE_PLAY_END 14
#define STATE_HSPICK 15

// Master control function for this state
struct env_t;
int state_play(struct env_t * env);

#endif

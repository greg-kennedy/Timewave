/* hspick.h - high score colorpick */
#ifndef HSPICK_H_
#define HSPICK_H_

struct env_t;
typedef struct _Mix_Music Mix_Music;
int state_hspick(struct env_t* env, Mix_Music* music_pause, int score);

#endif

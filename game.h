/* game.h - gameplay header */
#ifndef GAME_H_
#define GAME_H_

struct env_t;
typedef struct _Mix_Music Mix_Music;

// Because there are expensive objects on this screen, provide additional
//  init_game and shutdown_game methods to trigger preloading.
int init_game(struct env_t* env);
void free_game(void);

int state_game(struct env_t* env, Mix_Music* music_pause, int level, int* score);

#endif

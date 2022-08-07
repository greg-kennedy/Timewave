/* winlose.h - victory and game over screens */
#ifndef WINLOSE_H_
#define WINLOSE_H_

struct env_t;

int state_victory(struct env_t* env, int score);
int state_gameover(struct env_t* env, int score);

#endif

/* meta_ui.h - UI loop */
#ifndef META_UI_H_
#define META_UI_H_

/* *************************************************** */
// Local state numbers
#define STATE_TITLE 10
#define STATE_HS 11
#define STATE_MENU 12
#define STATE_KEYCONF 13

// Master control function for this state
struct env_t;
int state_ui(struct env_t * env, int arcade_mode);

#endif

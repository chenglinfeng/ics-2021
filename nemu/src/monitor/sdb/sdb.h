#ifndef __SDB_H__
#define __SDB_H__

#include <common.h>

word_t expr(char *e, bool *success);

/* watcpoint */

void add_wp(char* expr_c, bool* success);
bool delete_wp(int NO);
void wp_display();

#endif

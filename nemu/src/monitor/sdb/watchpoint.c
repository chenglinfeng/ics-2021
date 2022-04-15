#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

  char expr[64];  // the expression watched
  word_t last_val;// last value of the expression

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char* expr_c) {
  assert(free_);
  WP* new = free_;
  free_ = free_->next;
  strcpy(new->expr, expr_c);
  return new;
}

void add_wp(char* expr_c, bool* success) {
  WP* new = new_wp(expr_c);
  new->next = head;
  new->last_val = expr(expr_c, success);
  head = new;
}

void free_wp(WP *wp) {
  wp->next = free_;
  free_= wp;
}

bool delete_wp(int NO) {
  WP* wp_NO = head;
  if (wp_NO->NO == NO) {
    head = wp_NO->next;
    free_wp(wp_NO);
    return true;
  }
  else {
    while(wp_NO->next && wp_NO->next->NO != NO) {
      wp_NO = wp_NO->next;
    }
    if (!wp_NO->next) return false;
    else {
      WP* temp = wp_NO->next;
      wp_NO->next = temp->next;
      free_wp(temp);
      return true;
    }
  }
}

void wp_display() {
  if (!head) {puts("No watchpoint yet"); return;}
  printf("%-20s%-20s\n", "Num", "What");
  WP* temp = head;
  while(temp){
    printf("%-20d%-20s\n", temp->NO, temp->expr);
    temp = temp->next;
  }
}

bool wp_update_display_changed() {
  WP* temp = head;
  bool flag = false;
  bool success;     // not used
  while(temp != NULL){
    word_t new_val = expr(temp->expr, &success);
    if (temp->last_val != new_val) {
      if(!flag) {flag = true; printf("Watchpoint value changed:\n");}
      printf("Watchpoint %2d: %-16s new:%-10lu(0x%08lx)      old:%-10lu(0x%08lx)\n", temp->NO, temp->expr, new_val, new_val, temp->last_val, temp->last_val);
      temp->last_val = new_val;
    }
    temp = temp->next;
  }
  if (flag) return true; else return false;
}
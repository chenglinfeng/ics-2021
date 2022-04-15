#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <memory/paddr.h>
#include "sdb.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT; // fix Error 1
  return -1;
}

static int cmd_si(char *args) {
  char* s_num = strtok(NULL, " ");
  if (s_num == NULL) {
    cpu_exec(1);
  } else {
    int num = atoi(s_num);
    cpu_exec(num);
  }
  return 0;
}

static int cmd_p(char *args)
{
  bool flag=true;
  int val = expr(args,&flag);
  printf("%d\n",val);
  if(!flag)
  {
    printf("Wrong format!\n");
    return -1;
  }
  return 0;
}

static int cmd_info(char *args) {
  char* op = strtok(NULL, " ");
  if (op == NULL) {
    return 0;
  }
  if (strcmp(op, "r") == 0) {
    // 直接调用就好了, 因为这个是ISA相关的, 所以通过一个接口来对上层屏蔽差异
    isa_reg_display();
    return 0;
  }else if(strcmp(op, "w") == 0){
      /* print the information of watch points */
      wp_display();
    }
  return 0;
}

static int cmd_x(char *args) {
  char* s_num1 = strtok(NULL, " ");
  if (s_num1 == NULL) {
    return 0;
  }
  int num1 = atoi(s_num1);
  char* s_num2 = strtok(NULL, " ");
  if (s_num2 == NULL) {
    return 0;
  }
  if (strlen(s_num2) <= 2) {
    Assert(0, "x的第二个参数必须以0x开头,怎么会长度<=2呢?");
  }
  // 因为开头有一个0x, 我们需要去掉它, 不然解析会出错
  // BUG here: atoi默认是十进制, 但是这里应该是十六进制
  // paddr_t addr = atoi(s_num2+2);
  paddr_t addr = (paddr_t)strtol(s_num2+2, NULL, 16);
  // 开始扫描
  printf("%s\t\t%-34s%-32s\n", "addr", "16进制", "10进制");
  printf("%s:\t", s_num2);
  for (int i = 1; i <= num1<<2; i++) {
    // 因为这个是一个字节数组, 所以我们需要四个为一组进行扫描
    if (i%4 != 0) {
      printf("0x%-4lx ", paddr_read(addr + i - 1, 1));
    } else {
      printf("0x%-4lx\t", paddr_read(addr + i - 1, 1));
      for (int j = i - 3; j <= i; j++) {
        // 打印十进制的
        printf("%-4ld ", paddr_read(addr + j - 1, 1));
      }
      printf("\n");
      if (i == num1<<2) {
        printf("\n");
      } else {
        printf("0x%x:\t", addr + i);
      }
    }
  }
  return 0;
}

static int cmd_w(char* args) {
  bool success;
  add_wp(args, &success);
  if (!success) printf("Unvalid expression\n");
  return 0;
}

static int cmd_d(char* args) {
  int n;
  sscanf(args, "%d", &n);
  if (!delete_wp(n)) printf("Watchpoint %d does not exist\n", n);
  return 0;
}

static int cmd_exprtest(char* args) {

  FILE* input = fopen("tools/gen-expr/input", "r");
  char line[1024] = {};
  int try_count = 0;
  int error_count = 0;

  while(fgets(line, 512, input) != NULL) {
    try_count++;
    //printf("%s", line);
    char* t_res_s = strtok(line, " ");
    char* t_expr = strtok(NULL, " ");
    bool success;

    //printf("%s\n%s", t_res_s, t_expr);

    word_t res = expr(t_expr, &success);
    word_t t_res;
    sscanf(t_res_s, "%lu", &t_res);

    if (success && (res == t_res)) {
      printf("%d times: Correct\n", try_count);
    }
    else {
      error_count++;
      printf("%d times: Error\n", try_count);
      printf("Expression:%s\n given_result:%lu\tnemu_result:%lu\n", t_expr, t_res, res);
      return 0;
    }
  }
  printf("Total: %d errors\n", error_count);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  /* TODO: Add more commands */
  {"si", "Single step excution", cmd_si},
  {"info", "Show information", cmd_info},
  {"x", "scan memory", cmd_x},
  {"p", "p Expr", cmd_p},
  {"w", "Set a watchpoint to supervise the value of an expression", cmd_w},
  {"d", "Delete watchpoint N", cmd_d},
  {"exprtest", "To test the correctness of command p", cmd_exprtest},

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; 
        }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

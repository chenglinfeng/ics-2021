#include <cpu/cpu.h>
#include <cpu/exec.h>
#include <cpu/difftest.h>
#include <isa-all-instr.h>
#include <locale.h>

/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INSTR_TO_PRINT 10

#ifdef CONFIG_WATCHPOINT
  bool wp_update_display_changed();
#endif

#ifdef CONFIG_IRINGBUF
#define RINGBUF_SIZE 16
  uint8_t ring_count = -1;
  char ringbuf[RINGBUF_SIZE][128];
  void iringbuf_display() {
    for (int i = 0; i < RINGBUF_SIZE; i++) {
      if (i == ring_count) {
        printf("-->%s\n", ringbuf[i]);
        //log_write("-->%s\n", ringbuf[i]);
      }
      else {
        printf("   %s\n", ringbuf[i]);
        //log_write("   %s\n", ringbuf[i]);
      }
    }
  }
#endif

#ifdef CONFIG_FTRACE
  bool fflag;
  char fbuf[12800];
  #include<ftrace.h>
  int depth_count = 0;
#endif

CPU_state cpu = {};
uint64_t g_nr_guest_instr = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
const rtlreg_t rzero = 0;
rtlreg_t tmp_reg[4];

void device_update();
void fetch_decode(Decode *s, vaddr_t pc);

static void trace_and_difftest(Decode *_this, vaddr_t dnpc) {
#ifdef CONFIG_ITRACE_COND
  if (ITRACE_COND) log_write("%s\n", _this->logbuf);
#endif
#ifdef CONFIG_IRINGBUF
  if (nemu_state.state == NEMU_ABORT)
    iringbuf_display();
#endif
  if (g_print_step) { IFDEF(CONFIG_ITRACE, puts(_this->logbuf)); }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));

#ifdef CONFIG_WATCHPOINT
  if (wp_update_display_changed()) nemu_state.state = NEMU_STOP;
#endif

#ifdef CONFIG_FTRACE
  if (fflag) {
    printf("%x:",_this->pc);
    puts(fbuf);
    log_write("%x:%s\n", _this->pc, fbuf);
  }
#endif

}

#include <isa-exec.h>

#define FILL_EXEC_TABLE(name) [concat(EXEC_ID_, name)] = concat(exec_, name),
static const void* g_exec_table[TOTAL_INSTR] = {
  MAP(INSTR_LIST, FILL_EXEC_TABLE)
};

static void fetch_decode_exec_updatepc(Decode *s) {
  fetch_decode(s, cpu.pc);
  s->EHelper(s);
#ifdef CONFIG_FTRACE
  char* pf = fbuf;
  int opcode = s->isa.instr.val & 0x7f;
  if (opcode == 0b1101111 || opcode == 0b1100111) {
    fflag = true;
    if (s->isa.instr.val == 0x00008067) {
      memset(pf, ' ', depth_count);
      pf += depth_count;
      for (int i = 0; i < functab_num; i++) {
        if (s->pc >= functab[i].st_value && s->pc < functab[i].st_value + functab[i].st_size) {
          pf += sprintf(pf, "ret[%s]", idx2str(strtab, functab[i].st_name));
          depth_count--;
          break;
        }
      }
      *pf = 0;
    }
    else {
      memset(pf, ' ', depth_count);
      pf += depth_count;
      for (int i = 0; i < functab_num; i++) {
        if (s->dnpc >= functab[i].st_value && s->dnpc < functab[i].st_value + functab[i].st_size) {
          pf += sprintf(pf, "call[%s@0x%08x]", idx2str(strtab, functab[i].st_name), s->dnpc);
          depth_count++;
          break;
        }
      }
      *pf = 0;
    }
  }
  else{
    fflag = false;
  }
#endif 
  cpu.pc = s->dnpc;
}

static void statistic() {
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%ld", "%'ld")
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_instr);
  if (g_timer > 0) Log("simulation frequency = " NUMBERIC_FMT " instr/s", g_nr_guest_instr * 1000000 / g_timer);
  else Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

void assert_fail_msg() {
  isa_reg_display();
#ifdef CONFIG_IRINGBUF
  iringbuf_display();
#endif
  statistic();
}

void fetch_decode(Decode *s, vaddr_t pc) {
  s->pc = pc;
  s->snpc = pc;
  int idx = isa_fetch_decode(s);
  s->dnpc = s->snpc;
  s->EHelper = g_exec_table[idx];
#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *instr = (uint8_t *)&s->isa.instr.val;
  for (i = 0; i < ilen; i ++) {
    p += snprintf(p, 4, " %02x", instr[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0) space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
      MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.instr.val, ilen);
#endif

#ifdef CONFIG_IRINGBUF
  ring_count = (ring_count + 1) % RINGBUF_SIZE;
  strcpy(ringbuf[ring_count], s->logbuf);
#endif

}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n) {
  g_print_step = (n < MAX_INSTR_TO_PRINT);
  switch (nemu_state.state) {
    case NEMU_END: case NEMU_ABORT:
      printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
      return;
    default: nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  Decode s;
  for (;n > 0; n --) {
    fetch_decode_exec_updatepc(&s);
    g_nr_guest_instr ++;
    trace_and_difftest(&s, cpu.pc);
    if (nemu_state.state != NEMU_RUNNING) break;
    IFDEF(CONFIG_DEVICE, device_update());
  }

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state) {
    case NEMU_RUNNING: nemu_state.state = NEMU_STOP; break;

    case NEMU_END: case NEMU_ABORT:
#ifdef CONFIG_FTRACE
      release_ftrace();
#endif    
      Log("nemu: %s at pc = " FMT_WORD,
          (nemu_state.state == NEMU_ABORT ? ASNI_FMT("ABORT", ASNI_FG_RED) :
           (nemu_state.halt_ret == 0 ? ASNI_FMT("HIT GOOD TRAP", ASNI_FG_GREEN) :
            ASNI_FMT("HIT BAD TRAP", ASNI_FG_RED))),
          nemu_state.halt_pc);
      // fall through
    case NEMU_QUIT: statistic();
  }
}

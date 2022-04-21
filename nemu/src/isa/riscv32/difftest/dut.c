#include <isa.h>
#include <macro.h>
#include <cpu/difftest.h>
#include "../local-include/reg.h"

bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
    if (ref_r->pc != cpu.pc) {
    Log("pc is different after executing instruction at pc = " FMT_WORD
        ", right = " FMT_WORD ", wrong = " FMT_WORD, pc, ref_r->pc, cpu.pc);
    return false;
  }

  for (int i = 0; i < 32; i++) {
    if (!difftest_check_reg(reg_name(i, 0), pc, ref_r->gpr[i]._32, gpr(i)))
      return false;
  }
  return true;
}

void isa_difftest_attach() {
}

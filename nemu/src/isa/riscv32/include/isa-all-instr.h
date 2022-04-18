#include <cpu/decode.h>
#include "../local-include/rtl.h"

#define INSTR_LIST(f) f(add) f(sub) \
                      f(mul) f(mulh) f(mulhu) f(div) f(divu) f(rem) f(remu) \
                      f(and) f(or) f(xor) \
                      f(sll) f(srl) f(sra) \
                      f(slt) f(sltu) \
                      f(addi) f(andi) f(ori) f(xori) \
                      f(slti) f(sltiu) \
                      f(slli) f(srli) f(srai) \
                      f(lui) f(auipc) \
                      f(jal) f(jalr) \
                      f(lw) f(lh) f(lhu) f(lb) f(lbu) \
                      f(sw) f(sh) f(sb) \
                      f(beq) f(bne) f(bge) f(bgeu) f(blt) f(bltu) \
                      f(inv) f(nemu_trap)

def_all_EXEC_ID();

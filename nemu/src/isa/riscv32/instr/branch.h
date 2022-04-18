def_EHelper(beq) {
    rtl_addi(s, s0, &s->pc, id_dest->simm);
    rtl_jrelop(s, RELOP_EQ, dsrc1, dsrc2, *s0);
}

def_EHelper(bne) {
    rtl_addi(s, s0, &s->pc, id_dest->simm);
    rtl_jrelop(s, RELOP_NE, dsrc1, dsrc2, *s0);
}

def_EHelper(bge) {
    rtl_addi(s, s0, &s->pc, id_dest->simm);
    rtl_jrelop(s, RELOP_GE, dsrc1, dsrc2, *s0);
}

def_EHelper(bgeu) {
    rtl_addi(s, s0, &s->pc, id_dest->simm);
    rtl_jrelop(s, RELOP_GEU, dsrc1, dsrc2, *s0);
}

def_EHelper(blt) {
    rtl_addi(s, s0, &s->pc, id_dest->simm);
    rtl_jrelop(s, RELOP_LT, dsrc1, dsrc2, *s0);
}

def_EHelper(bltu) {
    rtl_addi(s, s0, &s->pc, id_dest->simm);
    rtl_jrelop(s, RELOP_LTU, dsrc1, dsrc2, *s0);
}

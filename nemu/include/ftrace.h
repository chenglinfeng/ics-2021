#include<elf.h>

extern Elf32_Sym* functab;
extern int functab_num;
extern char* strtab;

char* idx2str(char* tab, int idx);
void release_ftrace();
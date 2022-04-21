#include<ftrace.h>
#include<common.h>

#define STRTAB_NAME   ".strtab"
#define SHSTRTAB_NAME ".shstrtab"
#define SYMTAB_NAME   ".symtab"

char* elf;         // .elf file
Elf32_Ehdr* hdr;   // header
Elf32_Shdr* shdr;  // section header
Elf32_Sym* symtab; // symbol table
int symtab_num;    // length of symtab
Elf32_Sym* functab;// function table
int functab_num;   // function table length
char* SHstrtab;    // section header string table
char* strtab;      // string table

static void get_elf_file(const char* elf_file) {
    FILE* fp;
    fp = fopen(elf_file, "r");
    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    elf = malloc(fsize);
    fseek(fp, 0,SEEK_SET);
    assert(fread(elf, fsize, 1, fp) == 1);
    fclose(fp);
}

static void get_header() {
    hdr = (Elf32_Ehdr*)elf;
}

static void get_shdr() {
    shdr = (Elf32_Shdr*)(elf + hdr->e_shoff);
}

static void get_SHstrtab() {
    for (int i = 0; i < hdr->e_shnum; i++) {
        if (shdr[i].sh_type == SHT_STRTAB) {
            if (shdr[i].sh_name >= shdr[i].sh_size) 
                continue;
            else if (strcmp(idx2str(elf + shdr[i].sh_offset, shdr[i].sh_name), SHSTRTAB_NAME) == 0) {
                SHstrtab = elf + shdr[i].sh_offset;
                return;
            }
        }
    }
}

static void get_strtab() {
    for (int i = 0; i < hdr->e_shnum; i++) {
        if (strcmp(idx2str(SHstrtab, shdr[i].sh_name), STRTAB_NAME) == 0) {
            strtab = elf + shdr[i].sh_offset;
            return;
        }
    } 
}

static void get_symtab() {
    for (int i = 0; i < hdr->e_shnum; i++) {
        if (strcmp(idx2str(SHstrtab, shdr[i].sh_name), SYMTAB_NAME) == 0) {
            symtab = (Elf32_Sym*)(elf + shdr[i].sh_offset);
            symtab_num = shdr[i].sh_size / sizeof(Elf32_Sym);
        }
    }
}

static void get_functab() {
    functab = malloc(sizeof(Elf32_Sym) * symtab_num);
    functab_num = 0;
    for(int i = 0; i < symtab_num; i++) {
        if (ELF32_ST_TYPE(symtab[i].st_info) == STT_FUNC) {
            functab[functab_num] = symtab[i];
            functab_num++;
        }
    }
}

char* idx2str(char* tab, int idx) {
    return tab + idx;
}

void init_ftrace(const char* elf_file) {

    /* load .elf file */
    get_elf_file(elf_file);

    /* read header */
    get_header();

    /* read section table */
    get_shdr();
    
    /* read the symbol table and string table */
    get_SHstrtab();
    get_strtab();
    get_symtab();
    
    /* get the func part */
    get_functab();
}

void release_ftrace() {
    free(elf);
    free(functab);
}
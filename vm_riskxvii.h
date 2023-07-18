#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1
#define NUM_OF_REGS 32
#define NUM_OF_VR 256
#define NUM_OF_HB 128

#define INST_MEM_SIZE 1024
#define DATA_MEM_SIZE 1024


struct blob {
    char inst_mem[INST_MEM_SIZE];
    char data_mem[DATA_MEM_SIZE];
};

struct heap{
    unsigned char allocated;
    unsigned char start; // is this a starting heap
    unsigned int start_size; // max 8192.
    unsigned int size; // max 64;
    char* alloc_mem;
};

void virtual_routines(int* address, int* stored_value, unsigned int* pc,
    int* registers, struct blob* file_memory, struct heap* heap_banks);

void allocate_mem(int* size, int* registers, 
    struct heap* heap_banks);

void free_mem(int* address, struct heap* heap_banks, 
    struct blob* file_memory, unsigned int* pc, 
    int* registers);

void free_all(struct heap* heap_banks);

void init_heap_banks(struct heap* heap_banks);

void get_heap_pos(struct heap* heap_banks, int* idx, 
    struct heap** found, int* address, int add_size);

unsigned int power(unsigned int a, unsigned int b);

void comm_not_imp(struct blob* file_memory, unsigned int* pc, 
    int* registers);

void illegal_op(struct blob* file_memory, unsigned int* pc, 
    int* registers);

void reg_dump(int* registers);

void pc_dump(unsigned int* pc);

void sb_command(struct blob* file_memory, unsigned int* pc,
    int* rs1, int* rs2, int* imm, int* registers,
    struct heap* heap_banks);

void sh_command(struct blob* file_memory, unsigned int* pc,
    int* rs1, int* rs2, int* imm, int* registers,
    struct heap* heap_banks);

void sw_command(struct blob* file_memory, unsigned int* pc,
    int* rs1, int* rs2, int* imm, int* registers,
    struct heap* heap_banks);

void lb_command(struct blob* file_memory, unsigned int* pc,
    int* rd, int* rs1, int* imm, int* registers,
    struct heap* heap_banks, int u);

void lh_command(struct blob* file_memory, unsigned int* pc,
    int* rd, int* rs1, int* imm, int* registers, 
    struct heap* heap_banks, int u);

void lw_command(struct blob* file_memory, unsigned int* pc,
    int* rd, int* rs1, int* imm, int* registers, 
    struct heap* heap_banks);

void sra_command(unsigned int rs1, unsigned int rs2, unsigned int* rd);

void get_func3(struct blob* file_memory, unsigned int* pc, 
    unsigned char* func3);

void get_func7(struct blob* file_memory, unsigned int* pc, 
    unsigned char* func7);

void get_regs(struct blob* file_memory, unsigned int* pc, 
    unsigned char* r, int shift, int idx);

void get_imm_i(struct blob* file_memory, unsigned int* pc, 
    int* imm);

void get_imm_s(struct blob* file_memory, unsigned int* pc, 
    int* imm);

void get_imm_sb(struct blob* file_memory, unsigned int* pc, 
    int* imm);

void get_imm_u(struct blob* file_memory, unsigned int* pc, 
    int* imm);

void get_imm_uj(struct blob* file_memory, unsigned int* pc, 
    int* imm);

void comb_4_bytes(unsigned int* b1, unsigned int* b2, 
    unsigned int* b3, unsigned int* b4, 
    unsigned int* container);

void comb_2_bytes(unsigned short int* b1, unsigned short int* b2,
    unsigned short int* container);

void print_char_bin(char target, char end);

void printBits(size_t const size, void const * const ptr, 
    char* end, char* label, int use_label);

void printCommandBits(struct blob* file_memory, unsigned int* pc);
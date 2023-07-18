#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "vm_riskxvii.h"

int main(int argc, char** argv) {

    // check if there's an arg for the binary file.
    if (argc < 2) {
        printf("Not enough arguments!\n");
        return 1;
    }

    //open the binary file.
    char* file_path = argv[1];
    char buffer[BUF_SIZE];

    FILE* f = fopen(file_path, "rb");

    if (f == NULL) {
        printf("FIle %s does not exist!\n", file_path);
        return 2;
    }

    //initialise the blob for storing the binary file.
    struct blob file_memory;
    
    int ret = 0;
    int j = 0;

    for (int i = 0; i < INST_MEM_SIZE + DATA_MEM_SIZE; i++) {
        ret = fread(buffer, BUF_SIZE, 1, f);

        if (ret == 0)
            break;
        
        if (i < INST_MEM_SIZE) {
            file_memory.inst_mem[i] = *buffer;
        } else {
            file_memory.data_mem[j] = *buffer;
            j++;
        }
    }
    
    fclose(f);

    // initialise program counter
    unsigned int pc = 0;

    // initialise registers
    int registers[NUM_OF_REGS];

    for (int i = 0; i < NUM_OF_REGS; i++) {
        registers[i] = 0;
    }

    // initialise the heap banks
    struct heap heap_banks[NUM_OF_HB];
    init_heap_banks(heap_banks);
    
    while(1) {
        
        if (pc >= INST_MEM_SIZE)
            break;
            
        // change back register 0 into 0
        registers[0] = 0;

        char op_code = file_memory.inst_mem[pc];
        op_code = op_code & 0x7f;

        if (op_code == 51) {

            // this instruction is of type R.
            unsigned char func3 = 0;
            unsigned char func7 = 0;

            get_func3(&file_memory, &pc, &func3);
            get_func7(&file_memory, &pc, &func7);

            // get registers.
            unsigned char rd = 0;
            unsigned char rs1 = 0;
            unsigned char rs2 = 0;

            get_regs(&file_memory, &pc, &rd, 7, 0);
            get_regs(&file_memory, &pc, &rs1, 7, 1);
            get_regs(&file_memory, &pc, &rs2, 4, 2);

            if (func3 == 0 && func7 == 0) {

                // add
                registers[rd] = registers[rs1] + registers[rs2];

            } else if (func3 == 0 && func7 == 32) {

                // sub
                registers[rd] = registers[rs1] - registers[rs2];

            } else if (func3 == 4 && func7 == 0) {

                // xor
                registers[rd] = registers[rs1] ^ registers[rs2];

            } else if (func3 == 6 && func7 == 0) {

                // or
                registers[rd] = registers[rs1] | registers[rs2];

            } else if (func3 == 7 && func7 == 0) {

                // and
                registers[rd] = registers[rs1] & registers[rs2];

            } else if (func3 == 1 && func7 == 0) {

                // sll
                if (registers[rs1] < 0 || registers[rs2] < 0) {
                    
                    registers[rd] = (unsigned int) registers[rs1] \
                    << (unsigned int) registers[rs2];

                } else {
                    registers[rd] = registers[rs1] << registers[rs2];
                }

            } else if (func3 == 5 && func7 == 0) {

                // srl
                if (registers[rs1] < 0 || registers[rs2] < 0) {

                    registers[rd] = (unsigned int) registers[rs1] \
                    >> (unsigned int) registers[rs2];

                } else {
                    registers[rd] = registers[rs1] >> registers[rs2];
                }
                
                
            } else if (func3 == 5 && func7 == 32) {

                // sra
                sra_command(registers[rs1], registers[rs2], 
                    (unsigned int*) &(registers[rd]));

            } else if (func3 == 2 && func7 == 0) {

                // slt
                if (registers[rs1] < registers[rs2]) {
                    registers[rd] = 1;
                } else {
                    registers[rd] = 0;
                }
 
            } else if (func3 == 3 && func7 == 0) {

                // sltu
                if ((unsigned int) registers[rs1] 
                    < (unsigned int) registers[rs2]) {
                    registers[rd] = 1;
                } else {
                    registers[rd] = 0;
                }

            } else {
                // unrecognised command
                comm_not_imp(&file_memory, &pc, registers);
            }

        } else if (op_code == 19) {
            
            unsigned char func3 = 0;
            get_func3(&file_memory, &pc, &func3);

            int imm = 0;
            get_imm_i(&file_memory, &pc, &imm);

            unsigned char rd = 0;
            unsigned char rs1 = 0;

            get_regs(&file_memory, &pc, &rd, 7, 0);
            get_regs(&file_memory, &pc, &rs1, 7, 1);
            if (func3 == 0) {

                // addi
                registers[rd] = registers[rs1] + imm;

            } else if (func3 == 4) {

                // xori
                registers[rd] = registers[rs1] ^ imm;

            } else if (func3 == 6) {

                // ori
                registers[rd] = registers[rs1] | imm;

            } else if (func3 == 7) {
                
                // andi
                registers[rd] = registers[rs1] | imm;

            } else if (func3 == 2) {

                // slti
                if (registers[rs1] < imm) {
                    registers[rd] = 1;
                } else {
                    registers[rd] = 0;
                }

            } else if (func3 == 3) {

                // sltiu
                if ((unsigned int) registers[rs1] 
                    < (unsigned int) imm) {
                    registers[rd] = 1;
                } else {
                    registers[rd] = 0;
                }
                printf("sltiu\n");
                printf("\n");

            } else {
                // unrecognised command
                comm_not_imp(&file_memory, &pc, registers);
            }
        } else if (op_code == 55) {

            // lui: R[rd] = {31:12 = imm | 11:0 = 0}
            unsigned char rd = 0;
            get_regs(&file_memory, &pc, &rd, 7, 0);

            int imm = 0;
            get_imm_u(&file_memory, &pc, &imm);

            registers[rd] = imm;

        } else if (op_code == 3) {
            
            unsigned char func3 = 0;

            get_func3(&file_memory, &pc, &func3);

            unsigned char rd = 0;
            unsigned char rs1 = 0;

            get_regs(&file_memory, &pc, &rd, 7, 0);
            get_regs(&file_memory, &pc, &rs1, 7, 1);

            int imm = 0;
            get_imm_i(&file_memory, &pc, &imm);

            // lh, lw, lhu are assuming that the taking 16 bytes 
            // (or anything above 8) is taking the bytes after 
            // the first idx and then assemble them just like 
            // inst bytes: the first byte is the least
            // significant byte.

            if (func3 == 0) {

                // lb
                lb_command(&file_memory, &pc, &(registers[rd]), 
                    &(registers[rs1]), &imm, registers, 
                    heap_banks, 0);

            } else if (func3 == 1) {
                
                // lh
                lh_command(&file_memory, &pc, &(registers[rd]), 
                    &(registers[rs1]), &imm, registers, 
                    heap_banks, 0);

            } else if (func3 == 2) {

                // lw
                lw_command(&file_memory, &pc, &(registers[rd]), 
                    &(registers[rs1]), &imm, registers, 
                    heap_banks);

            } else if (func3 == 4) {

                // lbu
                lb_command(&file_memory, &pc, &(registers[rd]), 
                    &(registers[rs1]), &imm, registers, 
                    heap_banks, 1);

            } else if (func3 == 5) {

                // lhu
                lh_command(&file_memory, &pc, &(registers[rd]), 
                    &(registers[rs1]), &imm, registers, 
                    heap_banks, 1);

            } else {
                // unrecognised command
                comm_not_imp(&file_memory, &pc, registers);
            }
        } else if (op_code == 35) {

            unsigned char func3 = 0;
            get_func3(&file_memory, &pc, &func3);

            unsigned char rs1 = 0;
            unsigned char rs2 = 0;

            get_regs(&file_memory, &pc, &rs1, 7, 1);
            get_regs(&file_memory, &pc, &rs2, 4, 2);

            int imm = 0;
            get_imm_s(&file_memory, &pc, &imm);

            if (func3 == 0) {

                // sb
                sb_command(&file_memory, &pc, &(registers[rs1]), 
                    &(registers[rs2]), &imm, registers, 
                    heap_banks);

            } else if (func3 == 1) {

                // sh
                sh_command(&file_memory, &pc, &(registers[rs1]), 
                    &(registers[rs2]), &imm, registers, 
                    heap_banks);
            } else if (func3 == 2) {

                // sw
                sw_command(&file_memory, &pc, &(registers[rs1]), 
                    &(registers[rs2]), &imm, registers,
                    heap_banks);

            } else {
                // unrecognised command
                comm_not_imp(&file_memory, &pc, registers);
            }

        } else if (op_code == 99) {

            unsigned char func3 = 0;
            get_func3(&file_memory, &pc, &func3);

            unsigned char rs1 = 0;
            unsigned char rs2 = 0;

            get_regs(&file_memory, &pc, &rs1, 7, 1);
            get_regs(&file_memory, &pc, &rs2, 4, 2);

            int imm = 0;
            get_imm_sb(&file_memory, &pc, &imm);

            if (func3 == 0) {

                // beq
                if (registers[rs1] == registers[rs2]) {
                    pc  = pc + imm;
                    continue;
                }

            } else if (func3 == 1) {

                // bne
                if (registers[rs1] != registers[rs2]) {
                    pc  = pc + imm;
                    continue;
                }

            } else if (func3 == 4) {

                // blt
                if (registers[rs1] < registers[rs2]) {
                    pc  = pc + imm;
                    continue;
                }

            } else if (func3 == 6) {

                // bltu
                if ((unsigned int) registers[rs1] 
                    < (unsigned int) registers[rs2]) {

                    pc  = pc + imm;
                    continue;
                }

            } else if (func3 == 5) {

                // bge
                if (registers[rs1] >= registers[rs2]) {
                    pc  = pc + imm;
                    continue;
                }

            } else if (func3 == 7) {

                // bgeu
                if ((unsigned int) registers[rs1] 
                    >= (unsigned int) registers[rs2]) {

                    pc  = pc + imm;
                    continue;
                }

            } else {
                // unrecognised command
                comm_not_imp(&file_memory, &pc, registers);
            }

        } else if (op_code == 111) {

            unsigned char rd = 0;
            get_regs(&file_memory, &pc, &rd, 7, 0);

            int imm = 0;
            get_imm_uj(&file_memory, &pc, &imm);

            // jal
            registers[rd] = pc + 4;
            pc = pc + imm;
            continue;

        } else if (op_code == 103) {

            unsigned char func3 = 0;
            get_func3(&file_memory, &pc, &func3);

            unsigned char rd = 0;
            unsigned char rs1 = 0;

            get_regs(&file_memory, &pc, &rd, 7, 0);
            get_regs(&file_memory, &pc, &rs1, 7, 1);

            int imm = 0;
            get_imm_i(&file_memory, &pc, &imm);

            if (func3 == 0) {
                
                // jalr
                registers[rd] = pc + 4;
                pc = registers[rs1] + imm;
                continue;

            } else {
                // unrecognised command
                comm_not_imp(&file_memory, &pc, registers);
            }

        } else {
            // unrecognised command
            comm_not_imp(&file_memory, &pc, registers);
        }

        pc += 4;

    }

    return 0;

}

void virtual_routines(int* address, int* stored_value, unsigned int* pc,
    int* registers, struct blob* file_memory, struct heap* heap_banks) {

    if (*address == 0x0800) {
        printf("%c", *stored_value);

    } else if (*address == 0x0804) {
        printf("%d", *stored_value);

    } else if (*address == 0x0808) {
        printf("%x", (unsigned int) *stored_value);

    } else if (*address == 0x080C) {
        printf("CPU Halt Requested\n");
        exit(0);

    } else if (*address == 0x0812) {

        char input = 0;
        scanf("%c", &input);
        *stored_value = input;

    } else if (*address == 0x0816) {

        int input = 0;
        scanf("%d", &input);
        *stored_value = input;

    } else if (*address == 0x0820) {
        pc_dump(pc);

    } else if (*address == 0x0824) {
        reg_dump(registers);
        
    } else if (*address == 0x0828) {

        int idx = *stored_value - 1024;
        unsigned int b1 = file_memory->data_mem[idx];
        unsigned int b2 = file_memory->data_mem[idx+1];
        unsigned int b3 = file_memory->data_mem[idx+2];
        unsigned int b4 = file_memory->data_mem[idx+3];

        unsigned int container = 0;
        comb_4_bytes(&b1, &b2, &b3, &b4, &container);

        printf("0x%x", container);
    } else if (*address == 0x0830) {

        //malloc
        allocate_mem(stored_value, registers, heap_banks);

    } else if (*address == 0x0834) {

        //free
        free_mem(stored_value, heap_banks, file_memory, 
            pc, registers);

    }

}

void allocate_mem(int* size, int* registers, 
    struct heap* heap_banks) {
    
    unsigned int count = 0;
    unsigned int accumulated = 0;
    int idx = -1;

    // search for suitable banks.
    struct heap* h_ptr = heap_banks;
    for (int i = 0; i < NUM_OF_HB; i++) {

        if (h_ptr->allocated == 0) {
            count++;
            accumulated += 64;

            if (accumulated >= *size) {
                idx = i;
                break;
            }
        } else {
            count = 0;
            accumulated = 0;

        }
        h_ptr++;
    }

    // set reg 28 to zero and return if not found.
    if (idx == -1) {
        *(registers+28) = 0;
        return;
    }

    // set reg 28 to the starting address if found.
    idx = idx - count + 1;

    int temp = 0xb700 + (idx * 64);
    *(registers+28) = temp;

    // allocate memory
    
    int copy_size = *size;
    h_ptr = heap_banks + idx;

    h_ptr->allocated = 1;
    h_ptr->start = 1;
    h_ptr->start_size = copy_size;

    for (int i = idx; i < NUM_OF_HB; i++) {

        char* address = NULL;
        if (copy_size < 64) {

            address = (char*) malloc(sizeof(char)*(copy_size));
            h_ptr->allocated = 1;
            h_ptr->alloc_mem = address;
            h_ptr->size = copy_size;

            break;

        } else {
            address = (char*) malloc(sizeof(char)*64);
        }

        copy_size -= 64;

        h_ptr->allocated = 1;
        h_ptr->alloc_mem = address;
        h_ptr->size = 64;

        h_ptr++;
    }

}

void free_mem(int* address, struct heap* heap_banks, 
    struct blob* file_memory, unsigned int* pc, 
    int* registers) {

    if (*address < 0xb700 || *address > 0xD6c0) {
        // illegal memory operation
        // because outside memory boundaries.
        illegal_op(file_memory, pc, registers);
    }

    int idx = *address - 0xb700;

    if (idx % 64 != 0) {
        // illegal memroy operation
        // because not the start of a heap bank
        illegal_op(file_memory, pc, registers);
    }

    idx = idx / 64;
    struct heap* start_heap = heap_banks+idx;

    if (start_heap->start == 0) {
        // illegal memory operation
        // bcs its not the start of the allocate mem
        illegal_op(file_memory, pc, registers);
    }

    start_heap->start = 0;
    int size = start_heap->start_size;

    struct heap* h_ptr = heap_banks;
    for (int i = 0; i < NUM_OF_HB; i++) {

        free(h_ptr->alloc_mem);
        h_ptr->allocated = 0;
        
        if (size < 64)
            break;
        
        size -= 64;

        h_ptr++;
    }
}

void free_all(struct heap* heap_banks) {

    struct heap* ptr = heap_banks;
    for (int i = 0; i < NUM_OF_HB; i++) {

        if (ptr->allocated == 1)
            free(ptr->alloc_mem);

        ptr++;
    }
}

void init_heap_banks(struct heap* heap_banks) {

    struct heap* ptr = heap_banks;
    for (int i = 0; i < NUM_OF_HB; i++) {
        
        ptr->allocated = 0;
        ptr->size = 0;
        ptr->start = 0;

        ptr++;
    }
}

void get_heap_pos(struct heap* heap_banks, int* idx, 
    struct heap** found, int* address, int add_size) {

    // assume the address is within the boundary
    // of the heap address.

    int pos = *address - 0xb700;

    int temp_idx = pos % 64;
    pos = pos / 64;

    struct heap* temp_found = heap_banks+pos;
    if (temp_found->allocated == 0 
        || temp_idx+add_size >= temp_found->size) {

        // not allocated
        *idx = -1;
        *found = NULL;
        return;
    }
    
    *found = temp_found;
    *idx = temp_idx;

}
 
unsigned int power(unsigned int a, unsigned int b) {

    if (b == 0)
        return 1;

    b -= 1;
    unsigned int result = a;
    for (int i = 0; i < b; i++) {
        result = result * a;
    }
    return result;
}

void comm_not_imp(struct blob* file_memory, unsigned int* pc, 
    int* registers) {

    // get the bytes
    unsigned int b1 = file_memory->inst_mem[*pc];
    unsigned int b2 = file_memory->inst_mem[*pc+1];
    unsigned int b3 = file_memory->inst_mem[*pc+2];
    unsigned int b4 = file_memory->inst_mem[*pc+3];
    
    // combine the bytes
    unsigned int container = 0;
    comb_4_bytes(&b1, &b2, &b3, &b4, &container);

    printf("Instruction Not Implemented: 0x%x\n", container);
    pc_dump(pc);
    reg_dump(registers);
    exit(1);
}

void illegal_op(struct blob* file_memory, unsigned int* pc, 
    int* registers) {

    // get the bytes
    unsigned int b1 = file_memory->inst_mem[*pc];
    unsigned int b2 = file_memory->inst_mem[*pc+1];
    unsigned int b3 = file_memory->inst_mem[*pc+2];
    unsigned int b4 = file_memory->inst_mem[*pc+3];
    
    // combine the bytes
    unsigned int container = 0;
    comb_4_bytes(&b1, &b2, &b3, &b4, &container);

    printf("Illegal Operation: 0x%x\n", container);
    pc_dump(pc);
    reg_dump(registers);
    exit(1);
}

void reg_dump(int* registers) {

    int* ptr = registers;
    for (int i = 0; i < NUM_OF_REGS; i++) {

        printf("R[%d] = 0x%08x;\n", i, (unsigned int) *(ptr));
        ptr++;
    }
}

void pc_dump(unsigned int* pc) {
    printf("PC = 0x%08x;\n", (unsigned int) *pc);
}

void sb_command(struct blob* file_memory, unsigned int* pc,
    int* rs1, int* rs2, int* imm, int* registers,
    struct heap* heap_banks) {
    
    
    int idx = *rs1 + *imm;

    // check if out of boundary
    if (idx < 0x400 || (idx > 0x8ff && idx < 0xb700)
        || idx > 0xd6ff) {
        
        illegal_op(file_memory, pc, registers);
    }

    // check if virtual routine
    if (idx >= 0x800 && idx <= 0x8ff) {
        virtual_routines(&idx, rs2, pc, registers, 
            file_memory, heap_banks);
        return;
    }

    if (idx >=0xb700 && idx <= 0xd6ff) {
        //accessing heap;
        int position = -1;
        struct heap* heap = NULL;
        get_heap_pos(heap_banks, &position, &heap, &idx, 0);

        if (position == -1) {
            illegal_op(file_memory, pc, registers);
        }
        *(heap->alloc_mem + position) = *rs2;

    } else {
        // accessing the data memory
        idx -= 1024;
        file_memory->data_mem[idx] = *rs2;
    }

}

void sh_command(struct blob* file_memory, unsigned int* pc,
    int* rs1, int* rs2, int* imm, int* registers, 
    struct heap* heap_banks) {

    int idx = *rs1 + *imm;

    // check if out of boundary
    if (idx < 0x400 || (idx > 0x8ff && idx < 0xb700)
        || idx > 0xd6fe) {
        
        illegal_op(file_memory, pc, registers);
    }

    // check if virtual routine
    if (idx >= 0x800 && idx <= 0x8ff) {

        virtual_routines(&idx, rs2, pc, registers, 
            file_memory, heap_banks);
        return;
    }

    // get the bytes
    unsigned int b1 = *rs2;
    unsigned int b2 = *rs2;

    // allign the bytes to the right most bits
    b1 = b1 & 0xff;
    b2 = b2 >> 8;
    b2 = b2 & 0xff;
    
    // store the bytes

    if (idx >=0xb700 && idx <= 0xd6fe) {
        //accessing heap;
        int position = -1;
        struct heap* heap = NULL;
        get_heap_pos(heap_banks, &position, &heap, &idx, 1);

        if (position == -1) {
            illegal_op(file_memory, pc, registers);
        }
        *(heap->alloc_mem + position) = (char) b1;
        *(heap->alloc_mem + position+1) = (char) b2;

    } else {
        // accessing the data memory
        idx -= 1024;
        file_memory->data_mem[idx] = (char) b1;
        file_memory->data_mem[idx+1] = (char) b2;
    }
}

void sw_command(struct blob* file_memory, unsigned int* pc,
    int* rs1, int* rs2, int* imm, int* registers, 
    struct heap* heap_banks) {
    
    int idx = *rs1 + *imm;

    // check if out of boundary
    if (idx < 0x400 || (idx > 0x8ff && idx < 0xb700)
        || idx > 0xd6fc) {
        
        illegal_op(file_memory, pc, registers);
    }

    // check if virtual routine
    if (idx >= 0x800 && idx <= 0x8ff) {

        virtual_routines(&idx, rs2, pc, registers, 
            file_memory, heap_banks);
        return;
    }

    // get the bytes
    unsigned int b1 = *rs2;
    unsigned int b2 = *rs2;
    unsigned int b3 = *rs2;
    unsigned int b4 = *rs2;

    // allign the bytes to the right most bits
    b1 = b1 & 0xff;
    b2 = b2 >> 8;
    b2 = b2 & 0xff;
    b3 = b3 >> 16;
    b3 = b3 & 0xff;
    b4 = b4 >> 24;
    b4 = b4 & 0xff;
    
    // store the bytes
    if (idx >=0xb700 && idx <= 0xd6fc) {
        
        //accessing heap;
        int position = -1;
        struct heap* heap = NULL;
        get_heap_pos(heap_banks, &position, &heap, &idx, 3);

        if (position == -1) {
            illegal_op(file_memory, pc, registers);
        }
        *(heap->alloc_mem + position) = (char) b1;
        *(heap->alloc_mem + position+1) = (char) b2;
        *(heap->alloc_mem + position+2) = (char) b3;
        *(heap->alloc_mem + position+3) = (char) b4;

    } else {
        // accessing the data memory
        idx -= 1024;
        file_memory->data_mem[idx] = (char) b1;
        file_memory->data_mem[idx+1] = (char) b2;
        file_memory->data_mem[idx+2] = (char) b3;
        file_memory->data_mem[idx+3] = (char) b4;
    }
}

void lb_command(struct blob* file_memory, unsigned int* pc,
    int* rd, int* rs1, int* imm, int* registers,
    struct heap* heap_banks, int u) {
    
    int idx = *rs1 + *imm;

    // check if out of boundary
    if (idx < 0x0 || (idx > 0x8ff && idx < 0xb700)
        || idx > 0xd6ff) {
        
        illegal_op(file_memory, pc, registers);
    }

    // check if virtual routines
    if (idx >= 0x800 && idx <= 0x8ff) {
        virtual_routines(&idx, rd, pc, registers, 
            file_memory, NULL);
        return;
    }

    unsigned int u_temp;
    int temp;

    if (idx >=0xb700 && idx <= 0xd6ff) {
        //accessing heap;
        int position = -1;
        struct heap* heap = NULL;
        get_heap_pos(heap_banks, &position, &heap, &idx, 0);

        if (position == -1) {
            illegal_op(file_memory, pc, registers);
        }
        u_temp = (unsigned char) *(heap->alloc_mem + position);
        temp = (char) *(heap->alloc_mem + position);

    } else {
        // accessing the data memory
        idx -= 1024;
        u_temp = (unsigned char) file_memory->data_mem[idx];
        temp = (char) file_memory->data_mem[idx];
        
    }

    if (u == 1) {
        *rd = u_temp;
    } else {
        // sign extend
        *rd = temp;
    }
    
    
}


void lh_command(struct blob* file_memory, unsigned int* pc,
    int* rd, int* rs1, int* imm, int* registers, 
    struct heap* heap_banks, int u) {

    int idx = *rs1 + *imm;

    // check if out of boundary
    if (idx < 0x0 || (idx > 0x8ff && idx < 0xb700)
        || idx > 0xd6fe) {
        
        illegal_op(file_memory, pc, registers);
    }    

    // check if virtual routines
    if (idx >= 0x800 && idx <= 0x8ff) {
        virtual_routines(&idx, rd, pc, registers, 
            file_memory, NULL);
        return;
    }

    // get the bytes
    unsigned short int b1 = 0;
    unsigned short int b2 = 0;

    if (idx >=0xb700 && idx <= 0xd6fe) {
        //accessing heap;
        int position = -1;
        struct heap* heap = NULL;
        get_heap_pos(heap_banks, &position, &heap, &idx, 0);

        if (position == -1) {
            illegal_op(file_memory, pc, registers);
        }
        
        b1 = *(heap->alloc_mem + position);
        b2 = *(heap->alloc_mem + position+1);

    } else {
        // accessing the data memory
        idx -= 1024;
        b1 = file_memory->data_mem[idx];
        b2 = file_memory->data_mem[idx+1];
        
    }
    
    // combine the bytes
    unsigned short int temp = 0;
    comb_2_bytes(&b1, &b2, &temp);

    if (u == 1) {
        unsigned int container = temp;
        *rd = container;
    } else {
        // sign extend
        int container = (short int) temp;
        *rd = container;
    }
}

void lw_command(struct blob* file_memory, unsigned int* pc,
    int* rd, int* rs1, int* imm, int* registers, 
    struct heap* heap_banks) {

    int idx = *rs1 + *imm;

    // check if out of boundary
    if (idx < 0x0 || (idx > 0x8ff && idx < 0xb700)
        || idx > 0xd6fc) {
        
        illegal_op(file_memory, pc, registers);
    }

    // check if virtual routines
    if (idx >= 0x800 && idx <= 0x8ff) {
        virtual_routines(&idx, rd, pc, registers, 
            file_memory, NULL);
        return;
    }

    // get the bytes
    unsigned int b1 = file_memory->data_mem[idx];
    unsigned int b2 = file_memory->data_mem[idx+1];
    unsigned int b3 = file_memory->data_mem[idx+2];
    unsigned int b4 = file_memory->data_mem[idx+3];

    if (idx >=0xb700 && idx <= 0xd6fe) {
        //accessing heap;
        int position = -1;
        struct heap* heap = NULL;
        get_heap_pos(heap_banks, &position, &heap, &idx, 0);

        if (position == -1) {
            illegal_op(file_memory, pc, registers);
        }
        
        b1 = *(heap->alloc_mem + position);
        b2 = *(heap->alloc_mem + position+1);
        b3 = *(heap->alloc_mem + position+2);
        b4 = *(heap->alloc_mem + position+3);

    } else {
        // accessing the data memory
        idx -= 1024;
        b1 = file_memory->data_mem[idx];
        b2 = file_memory->data_mem[idx+1];
        b3 = file_memory->data_mem[idx+2];
        b4 = file_memory->data_mem[idx+3];
        
    }
    
    // combine the bytes
    unsigned int container = 0;
    comb_4_bytes(&b1, &b2, &b3, &b4, &container);
    *rd = b1;

}

void sra_command(unsigned int rs1, unsigned int rs2, unsigned int* rd) {

    unsigned int container = 0;
    
    int shift = 31; //2,147,483,648
    unsigned int mask = power(2, shift);

    for (int i = 0; i < rs2; i++) {

        unsigned int bit_cont = rs1;

        bit_cont = bit_cont << shift;
        bit_cont = bit_cont & mask;

        container = container | bit_cont;
        rs1 = rs1 >> 1;
        rs1 = rs1 & (mask-1);

        shift--;
        mask = power(2, shift);
    }

    container = container | rs1;
    *rd = container;

}

void get_func3(struct blob* file_memory, unsigned int* pc, 
    unsigned char* func3) {
    
    // func3 should be in the sec byte.
    *func3 = file_memory->inst_mem[*pc + 1];

    // to remove the right side of func3
    *func3 = *func3 >> 4; 

    // to remove the left side of func3
    *func3 = *func3 & 0x7;
}

void get_func7(struct blob* file_memory, unsigned int* pc, 
    unsigned char* func7) {

    // func7 should be on the last byte.
    *func7 = file_memory->inst_mem[*pc + 3];

    // to remove the right side of func7
    *func7 = *func7 >> 1; 

    // remove the left side of func7
    *func7 = *func7 & 0x7f;
}


void get_regs(struct blob* file_memory, unsigned int* pc, 
    unsigned char* r, int shift, int idx) {

    unsigned short int b1 = file_memory->inst_mem[*pc + idx];
    unsigned short int b2 = file_memory->inst_mem[*pc + idx + 1];

    unsigned short int container = 0;
    comb_2_bytes(&b1, &b2, &container);

    // get the rd or rs1.
    container = container >> shift;
    container = container & 0x1f;

    *r = container;
}

void get_imm_i(struct blob* file_memory, unsigned int* pc, 
    int* imm) {

    unsigned short int b1 = file_memory->inst_mem[*pc + 2];
    unsigned short int b2 = file_memory->inst_mem[*pc + 3];

    unsigned short int container = 0;
    comb_2_bytes(&b1, &b2, &container);

    short int container2 = container;
    container2 = container2 >> 4;

    *imm = container2;
}

void get_imm_s(struct blob* file_memory, unsigned int* pc, 
    int* imm){
    
    // get the 4:0
    unsigned short int b1 = file_memory->inst_mem[*pc];
    unsigned short int b2 = file_memory->inst_mem[*pc + 1];

    unsigned short int container = 0;
    comb_2_bytes(&b1, &b2, &container);
    
    container = container >> 7;
    container = container & 0x1f;

    // get the 11:5
    b1 = file_memory->inst_mem[*pc + 3];
    b1 = b1 >> 1;
    b1 = b1 & 0x7f;

    // allign 11:5 with 4:0
    b1 = b1 << 5;
    b1 = b1 & 0xfe0;

    // merge them
    container = container | b1;

    // to preserve the value
    short int container2 = container;
    container2 = container2 << 4;
    container2 = container2 >> 4;

    *imm = container2;

}

void get_imm_sb(struct blob* file_memory, unsigned int* pc, 
    int* imm) {
    
    // combiner first and sec bytes to get 11 & 4:1
    unsigned short int b1 = file_memory->inst_mem[*pc];
    unsigned short int b2 = file_memory->inst_mem[*pc + 1];

    unsigned short int container = 0;
    comb_2_bytes(&b1, &b2, &container);

    container = container >> 7;
    container = container & 0x1f;

    // get the 11 and allign it
    unsigned short int elv = container & 0x1;
    elv = elv << 11;
    elv = elv & 0x800;

    // get the 4:1 (already alligned)
    container = container & 0x1e;

    // get the last byte for 12 & 10:5
    b1 = file_memory->inst_mem[*pc + 3];
    b1 = b1 >> 1;
    b1 = b1 & 0x7f;

    // get the 12 and allign it
    unsigned short int twlv = b1 & 0x40;
    twlv = twlv << 6;
    twlv = twlv & 0x1000;

    // get the 10:5 and allign it
    b1 = b1 & 0x3f;
    b1 = b1 << 5;
    b1 = b1 & 0x7e0;

    // merge them 
    container = container | b1 | elv | twlv;

    // to preserve the value
    short int container2 = container;
    container2 = container2 << 3;
    container2 = container2 >> 3;

    *imm = container2;
}

void get_imm_u(struct blob* file_memory, unsigned int* pc, 
    int* imm) {

    unsigned int sec_byte = file_memory->inst_mem[*pc + 1];
    unsigned int third_byte = file_memory->inst_mem[*pc + 2];
    unsigned int fourth_byte = file_memory->inst_mem[*pc + 3];

    // allign the 15:12
    sec_byte = sec_byte << 8;
    sec_byte = sec_byte & 0xf000;

    // allign the 23:16
    third_byte = third_byte << 16;
    third_byte = third_byte & 0xff0000;

    // allign the 31:24
    fourth_byte = fourth_byte << 24;
    fourth_byte = fourth_byte & 0xff000000;

    // merge them
    sec_byte = sec_byte | third_byte | fourth_byte;

    *imm = sec_byte;
}

void get_imm_uj(struct blob* file_memory, unsigned int* pc, 
    int* imm) {

    unsigned int sec_byte = file_memory->inst_mem[*pc + 1];
    unsigned int third_byte = file_memory->inst_mem[*pc + 2];
    unsigned int fourth_byte = file_memory->inst_mem[*pc + 3];

    // | 31-30-24 | 23-21-20-19-16 | 15-12-8 |
    // allign the 15:12
    sec_byte = sec_byte << 8;
    sec_byte = sec_byte & 0xf000;

    // take and allign the 19:16
    unsigned int container = third_byte << 16;
    container = container & 0xf0000;

    // combine 15:12 and 19:16 to become 19:12
    sec_byte = sec_byte | container;
    
    // take 11 and combine it with 19:12 to become 19:11
    container = third_byte << 7;
    container = container & 0x800;
    sec_byte = sec_byte | container;

    // get the 3:1 
    third_byte = third_byte >> 4;
    third_byte = third_byte & 0xe;

    // get 20
    container = fourth_byte << 13;
    container = container & 0x100000;

    // get 10:4 
    fourth_byte = fourth_byte << 4;
    fourth_byte = fourth_byte & 0x7f0;
    
    // merge 20, 19:11, 10:4, 3:1
    container = container | sec_byte | fourth_byte | third_byte;

    // to preserve the value (in case 1 is the 20th bit)
    int container2 = container;
    container2 = container2 << 11;
    container2 = container2 >> 11;

    *imm = container2;
}

void comb_4_bytes(unsigned int* b1, unsigned int* b2, 
    unsigned int* b3, unsigned int* b4, 
    unsigned int* container) {
    
    // allign the bytes then merge them
    *b1 = *b1 & 0xff;
    *b2 = *b2 << 8;
    *b2 = *b2 & 0xff00;
    *b3 = *b3 << 16;
    *b3 = *b3 & 0xff0000;
    *b4 = *b4 << 24;
    *b4 = *b4 & 0xff000000;

    *container = *b1 | *b2 | *b3 | *b4;
}

void comb_2_bytes(unsigned short int* b1, unsigned short int* b2,
    unsigned short int* container) {

    *container = 0;

    // combine the 2 bytes.
    *b2 = *b2 << 8;
    *b2 = *b2 & 0xff00;
    *b1 = *b1 & 0xff;
    *container = *container | *b1;
    *container = *container | *b2;

}

// I got this from: https://stackoverflow.com/questions
// /18327439/printing-binary-representation-of-a-char-in-c
// to help in testing.
void print_char_bin(char target, char end) {

    for (int i = 0; i < 8; i++) {
        printf("%d", !!((target << i) & 0x80));
    }
    printf("%c", end);
}

// I got this from: https://stackoverflow.com/questions/111928
// /is-there-a-printf-converter-to-print-in-binary-format
// to help in testing.
// Assumes little endian
void printBits(size_t const size, void const * const ptr, 
    char* end, char* label, int use_label)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;
    
    if (use_label == 1)
        printf("%s: ", label);

    for (i = size-1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            byte = (b[i] >> j) & 1;
            printf("%u", byte);
        }
    }
    printf("%s", end);
}

void printCommandBits(struct blob* file_memory, unsigned int* pc) {
    unsigned int b1 = file_memory->inst_mem[*pc];
    unsigned int b2 = file_memory->inst_mem[*pc+1];
    unsigned int b3 = file_memory->inst_mem[*pc+2];
    unsigned int b4 = file_memory->inst_mem[*pc+3];

    printf("PC: %d ", *pc);
    printBits(sizeof(char), &b4, " ", NULL, 0);
    printBits(sizeof(char), &b3, " ", NULL, 0);
    printBits(sizeof(char), &b2, " ", NULL, 0);
    printBits(sizeof(char), &b1, "\n", NULL, 0);
}
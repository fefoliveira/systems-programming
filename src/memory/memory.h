#ifndef MEMORY_H
#define MEMORY_H

#include "../isa.h"

#define MEMORY_SIZE 320
#define NUM_REGS 4

extern Instruction instruction_memory[MEMORY_SIZE];
extern int data_memory[MEMORY_SIZE];

void init_memory();
void print_instruction_memory();
void print_data_memory();

// a0, a1, a2, a3
extern int registers[NUM_REGS];

extern int PC;

#endif

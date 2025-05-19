#ifndef MEMORY_H
#define MEMORY_H

#include "../isa.h"

#define MEMORY_SIZE 320
#define NUM_REGS 4
#define INSTRUCTION_MEMORY_END 269 // 270 palavras para instruções
#define DATA_MEMORY_START 270 // 50 palavras para dados

struct typedef
{
	Instruction instr;
	int data;
}
Memory_cell;

external Memory_cell memory[MEMORY_SIZE];

void init_memory();
void print_instruction_memory();
void print_data_memory();

// a0, a1, a2, a3
extern int registers[NUM_REGS];

extern int PC;

#endif

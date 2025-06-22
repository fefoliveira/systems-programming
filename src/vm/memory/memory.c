// clang-format off
#include <stdio.h>
#include "memory.h"

Memory_cell memory[MEMORY_SIZE];

void init_memory()
{
	for (int i = 0; i < MEMORY_SIZE; i++) {
		memory[i].instr = NOP_INSTRUCTION;
		memory[i].data = 0;	
	}
}

const char *opcode_names[] = {
    "ADD",
    "SUB",
    "MUL",
    "DIV",
    "MV",
    "ST",
    "JMP",
    "JEQ",
    "JGT",
    "JLT",
    "W",
    "R",
    "STP"
};

void print_instruction_memory()
{
	printf("\nConteúdo na memória de instruções:\n");
	for (int i = 0; i <= INSTRUCTION_MEMORY_END; i++) {
		if (memory[i].instr.opcode != -1) {
			const char *opcode_str =
				(memory[i].instr.opcode >= 0 &&
				 memory[i].instr.opcode < NUM_OPCODES) ?
					opcode_names[memory[i].instr.opcode] : "UNKNOWN";
			printf("instr_mem[%d]: %s", i, opcode_str);
			if (memory[i].instr.op1 != -1) {
				printf(" mem[%d]", memory[i].instr.op1);
			}
			if (memory[i].instr.op2 != -1) {
				printf(" mem[%d]", memory[i].instr.op2);
			}
			if (memory[i].instr.op3 != -1) {
				printf(" mem[%d]", memory[i].instr.op3);
			}
			printf("\n");
		}
	}
	printf("\n");
}

void print_data_memory()
{
	printf("\nConteúdo na memória de dados:\n");
	for (int i = DATA_MEMORY_START; i < MEMORY_SIZE; i++) {
		if (memory[i].data != 0) {
			printf("data_mem[%d]: %d\n", i-270,
			       memory[i].data);
		}
	}
	printf("\n");
}
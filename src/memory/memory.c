// clang-format off
#include <stdio.h>
#include "memory.h"

int[] memory[MEMORY_SIZE];

void init_memory()
{
	for (int i = 0; i < MEMORY_SIZE; i++) {
		if (i <= INSTRUCTION_MEMORY_END) {
			memory[i] = NOP_INSTRUCTION;	
		} else {
			memory[i] = 0;
		}
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
	printf("\nConteúdo da memória de instruções:\n");
	for (int i = 0; i <= INSTRUCTION_MEMORY_END; i++) {
		if (instruction_memory[i].opcode != -1) {
			const char *opcode_str =
				(instruction_memory[i].opcode >= 0 &&
				 instruction_memory[i].opcode < NUM_OPCODES) ?
					opcode_names[instruction_memory[i].opcode] : "UNKNOWN";
			printf("instr_mem[%d]: %s", i, opcode_str);
			if (instruction_memory[i].op1 != -1) {
				printf(" mem[%d]", instruction_memory[i].op1);
			}
			if (instruction_memory[i].op2 != -1) {
				printf(" mem[%d]", instruction_memory[i].op2);
			}
			if (instruction_memory[i].op3 != -1) {
				printf(" mem[%d]", instruction_memory[i].op3);
			}
			printf("\n");
		}
	}
	printf("\n");
}

void print_data_memory()
{
	printf("Conteúdo da memória de dados:\n");
	for (int i = DATA_MEMORY_START; i < MEMORY_SIZE; i++) {
		if (data_memory[i] != 0) {
			printf("data_mem[%d]: %d\n", i,
			       data_memory[i]);
		}
	}
	printf("\n");
}
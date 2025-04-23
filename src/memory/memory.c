#include <stdio.h>
#include "memory.h"

Instruction instruction_memory[MEMORY_SIZE];
int data_memory[MEMORY_SIZE];

void init_memory()
{
	for (int i = 0; i < MEMORY_SIZE; i++) {
		instruction_memory[i] = NOP_INSTRUCTION;
		data_memory[i] = 0;
	}
}

void print_instruction_memory()
{
	printf("Conteúdo da memória de instruções:\n");
	for (int i = 0; i < MEMORY_SIZE; i++) {
		if (instruction_memory[i].opcode != -1) {
			printf("instruction_memory[%d]: Opcode %d, Op1 %d, Op2 %d, Op3 %d\n",
			       i, instruction_memory[i].opcode,
			       instruction_memory[i].op1,
			       instruction_memory[i].op2,
			       instruction_memory[i].op3);
		}
	}
	printf("\n");
}

void print_data_memory()
{
	printf("Conteúdo da memória de dados:\n");
	for (int i = 0; i < MEMORY_SIZE; i++) {
		if (data_memory[i] != 0) {
			printf("data_memory[%d]: Value %d\n", i, data_memory[i]);
		}
	}
	printf("\n");
}
#include <stdio.h>
#include <stdlib.h>
#include "../memory/memory.h"
#include "../isa.h"

int registers[NUM_REGS] = { 0 };
int PC = 0;

static void ula(Instruction instr)
{
	switch (instr.opcode) {
	case ADD:
		registers[instr.op1] =
			registers[instr.op2] + registers[instr.op3];
		break;
	case SUB:
		registers[instr.op1] =
			registers[instr.op2] - registers[instr.op3];
		break;
	case MUL:
		registers[instr.op1] =
			registers[instr.op2] * registers[instr.op3];
		break;
	case DIV:
		if (registers[instr.op3] == 0) {
			printf("Erro: divisão por zero na instrução %d\n", PC);
			exit(1);
		}
		registers[instr.op1] =
			registers[instr.op2] / registers[instr.op3];
		break;
	default:
		break;
	}
}

static void execute_memory(Instruction instr)
{
	switch (instr.opcode) {
	case MV:
		registers[instr.op1] =
			memory[DATA_MEMORY_START + instr.op2].data;
		break;
	case ST:
		memory[DATA_MEMORY_START + instr.op2].data =
			registers[instr.op1];
		break;
	default:
		break;
	}
}

static int controller(Instruction instr)
{
	switch (instr.opcode) {
	case JMP:
		PC = instr.op1;
		return 1;
	case JEQ:
		if (registers[instr.op1] == registers[instr.op2]) {
			PC = instr.op3;
			return 1;
		}
		break;
	case JGT:
		if (registers[instr.op1] > 0) {
			PC = instr.op3;
			return 1;
		}
		break;
	case JLT:
		if (registers[instr.op1] < 0) {
			PC = instr.op3;
			return 1;
		}
		break;
	default:
		break;
	}
	return 0;
}

static void io(Instruction instr)
{
	switch (instr.opcode) {
	case W:
		printf("\nOutput: %d\n",
		       memory[DATA_MEMORY_START + instr.op1].data);
		break;
	case R:
		printf("Input (mem[%d]): ", instr.op1);
		scanf("%d", &memory[DATA_MEMORY_START + instr.op1].data);
		break;
	default:
		break;
	}
}

int run_vm()
{
	while (1) {
		Instruction instr = memory[PC].instr;

		if (instr.opcode >= ADD && instr.opcode <= DIV) {
			ula(instr);
		} else if (instr.opcode == MV || instr.opcode == ST) {
			execute_memory(instr);
		} else if (instr.opcode >= JMP && instr.opcode <= JLT) {
			if (controller(instr))
				continue;
		} else if (instr.opcode == W || instr.opcode == R) {
			io(instr);
		} else if (instr.opcode == STP) {
			printf("Programa finalizado.\n");
			break;
		} else {
			printf("Instrução inválida no PC=%d\n", PC);
			break;
		}

		PC++;
		if (PC > INSTRUCTION_MEMORY_END) {
			fprintf(stderr,
				"O PC tentou ler mais palavras do que o limite existente na memória de instruções.");
			return 1;
		}
	}
}

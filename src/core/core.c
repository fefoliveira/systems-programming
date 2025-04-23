#include <stdio.h>
#include "../memory/memory.h"
#include "../isa.h"

int registers[NUM_REGS] = { 0 };
int PC = 0;

void run_vm()
{
	while (1) {
		Instruction instr = instruction_memory[PC];

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
				printf("Erro: divisão por zero na instrução %d\n",
				       PC);
				return;
			}
			registers[instr.op1] =
				registers[instr.op2] / registers[instr.op3];
			break;
		case MV:
			registers[instr.op1] = data_memory[instr.op2];
			break;
		case ST:
			data_memory[instr.op2] = registers[instr.op1];
			break;
		case JMP:
			PC = instr.op1;
			continue;
		case JEQ:
			if (registers[instr.op1] == registers[instr.op2]) {
				PC = instr.op3;
				continue;
			}
			break;
		case JGT:
			if (registers[instr.op1] > 0) {
				PC = instr.op3;
				continue;
			}
			break;
		case JLT:
			if (registers[instr.op1] < 0) {
				PC = instr.op3;
				continue;
			}
			break;
		case W:
			printf("Output: %d\n", data_memory[instr.op1]);
			break;
		case R:
			printf("Input (mem[%d]): ", instr.op1);
			scanf("%d", &data_memory[instr.op1]);
			break;
		case STP:
			printf("Programa finalizado.\n");
			return;
		default:
			printf("Instrução inválida no PC=%d\n", PC);
			return;
		}

		PC++;
	}
}

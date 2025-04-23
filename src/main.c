#include "memory/memory.h"

int main()
{
	init_memory();

	instruction_memory[0] = (Instruction){ ADD, 1, 2, 3 };
	instruction_memory[1] = (Instruction){ SUB, 4, 5, 6 };
	instruction_memory[2] = (Instruction){ MUL, 7, 8, 9 };
	instruction_memory[3] = (Instruction){ DIV, 10, 11, 12 };

	data_memory[0] = 100;
	data_memory[1] = 200;
	data_memory[2] = 300;

	print_instruction_memory();
	print_data_memory();

	return 0;
}
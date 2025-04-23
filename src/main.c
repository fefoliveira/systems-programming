#include "memory/memory.h"
#include "loader/loader.h"

int main()
{
	init_memory();
	load_program("./programs/program.txt");

	data_memory[0] = 10;
	data_memory[1] = 20;
	data_memory[2] = 0;

	print_instruction_memory();
	print_data_memory();

	return 0;
}
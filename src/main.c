#include "memory/memory.h"
#include "loader/loader.h"
#include "core/core.h"

int main()
{
	init_memory();
	load_program("./programs/program.txt");

	print_instruction_memory();
	// print_data_memory();

	run_vm();

	return 0;
}
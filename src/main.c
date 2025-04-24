#include "memory/memory.h"
#include "loader/loader.h"
#include "cpu/cpu.h"

int main()
{
	init_memory();
	int err = load_program("./programs/program.txt");
	if (err) {
		return 1;
	}

	print_instruction_memory();
	// print_data_memory();

	run_vm();

	return 0;
}
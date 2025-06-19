#include <stdio.h>
#include "memory/memory.h"
#include "loader/loader.h"
#include "cpu/cpu.h"

int main(int argc, char *argv[])
{
	if (argc != 2) {
		printf("argv[0]: %s\n", argv[0]);
		printf("argv[1]: %s\n", argv[1]);
		printf("Erro: Argumentos insuficientes.\n");
		return 1;
	}

	init_memory();

	char program_file[256];
	snprintf(program_file, sizeof(program_file), "programs/%s.txt",
		 argv[1]);
	int err = load_program(program_file);
	if (err) {
		return 1;
	}

	print_instruction_memory();

	run_vm();

	return 0;
}
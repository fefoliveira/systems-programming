#include <stdio.h>
#include <stdbool.h>
#include "../memory/memory.h"

bool load_program(const char *filename)
{
	FILE *file = fopen(filename, "r");
	if (!file) {
		perror("Erro ao abrir o arquivo");
		return 1;
	}
	printf("\nCarregando o programa: %s\n", filename);

	int opcode, op1, op2, op3;
	int line = 0;
	int i = 0;
	char buffer[256];

	while (fgets(buffer, sizeof(buffer), file)) {
		if (buffer[0] == '#' || buffer[0] == '\n') {
			continue; // Ignora comentários/linhas vazias
		}

		int count = sscanf(buffer, "%d %d %d %d", &opcode, &op1, &op2,
				   &op3);

		Instruction instr;

		switch (count) {
		case 4:
			instr = (Instruction){ opcode, op1, op2, op3 };
			break;
		case 3:
			instr = (Instruction){ opcode, op1, op2, -1 };
			break;
		case 2:
			instr = (Instruction){ opcode, op1, -1, -1 };
			break;
		case 1:
			instr = (Instruction){ opcode, -1, -1, -1 };
			break;
		default:
			fprintf(stderr,
				"Erro na linha %d: formato inválido: %s",
				line + 1, buffer);
			fclose(file);
			return 1;
		}

		if (i <= INSTRUCTION_MEMORY_END) {
			memory[i++].instr = instr;
		} else {
			fprintf(stderr,
				"O número de instruções ultrapassou o limite aceito");
			fclose(file);
			return 1;
		}
		line++;
	}

	fclose(file);
	return 0;
}

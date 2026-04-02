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

		// Linha de inicialização de dado: "d <índice> <valor>"
		if (buffer[0] == 'd' && buffer[1] == ' ') {
			int idx, val;
			if (sscanf(buffer + 2, "%d %d", &idx, &val) == 2) {
				int abs_idx = DATA_MEMORY_START + idx;
				if (abs_idx >= DATA_MEMORY_START && abs_idx < MEMORY_SIZE) {
					memory[abs_idx].data = val;
				} else {
					fprintf(stderr,
						"Erro na linha %d: índice de dado %d fora dos limites.\n",
						line + 1, idx);
					fclose(file);
					return 1;
				}
			}
			line++;
			continue;
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

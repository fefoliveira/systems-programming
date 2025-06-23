// clang-format off

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/utils.h"
#include "assembler.h"

/* --------------------------------------------------------------------------
   PASSAGEM 2:
   - Abre novamente o arquivo fonte
   - Para cada linha, gera números no arquivo objeto (.obj):
     > Se for pseudo-SPACE: escreve ‘0’ em N posições
     > Se for pseudo-CONST: escreve o valor da constante em 1 posição
     > Se for instrução normal: 
         * converte mnemônico → opcode, 
         * converte cada operando (label ou número literal) → valor numérico
         * escreve “opcode op1 op2 op3” (faltando operandos, coloca -1)
   - Usa a tabela de símbolos (construída na passagem 1) para resolver labels.
   -------------------------------------------------------------------------- */
bool second_pass(const char *src_filename, const char *obj_filename)
{
    FILE *fp_src = fopen(src_filename, "r");
    if (!fp_src) {
        perror("Erro ao abrir fonte na segunda passagem");
        return false;
    }
    FILE *fp_obj = fopen(obj_filename, "w");
    if (!fp_obj) {
        perror("Erro ao criar arquivo objeto");
        fclose(fp_src);
        return false;
    }

	char line[MAX_LINE_LEN];
    int line_num = 0;
    int loc = 0; // endereço corrente para escrever no arquivo objeto

	while (fgets(line, sizeof(line), fp_src) != NULL) {
        line_num++;
		char *ptr = line;
        trim(ptr);
        if (ptr[0] == '\0' || ptr[0] == '#') {
            continue;
        }

		// Se houver label, pula-o
        if (strchr(ptr, ':') != NULL) {
			// avança ptr para depois do “:”
            char *colon = strchr(ptr, ':');
            ptr = colon + 1;
            trim(ptr);
			if (ptr[0] == '\0') {
				// linha apenas com label; nada a gerar
				continue;
        }
		}

		// Extrai mnemônico
		char token[MAX_TOKEN_LEN] = { 0 };
		bool ok = (sscanf(ptr, "%s", token) == 1);
		if (!ok) {
            fprintf(stderr, "Linha %d: formato inválido.\n", line_num);
            fclose(fp_src);
            fclose(fp_obj);
            return false;
        }

		// Verifica se o token é pseudo-instrução
        PseudoType pseudo;
        if (is_pseudo(token, &pseudo)) {
            continue;
        } else {
			// É uma instrução normal
            int opc = lookup_opcode(token);
            if (opc < 0) {
                fprintf(stderr, "Linha %d: instrução \"%s\" inválida.\n", line_num, token);
                fclose(fp_src);
                fclose(fp_obj);
                return false;
            }
			int op1 = -1, op2 = -1, op3 = -1;

			int expected_operands = instr_table[opc].operand_count;
            ptr += strlen(token);
            trim(ptr);

			// Para cada operando:
			if (expected_operands >= 1) {
				char operand[MAX_TOKEN_LEN];
				bool ok = (sscanf(ptr, "%s", operand) == 1);
				if (!ok) {
                    fprintf(stderr, "Linha %d: faltou operando 1 em \"%s\".\n", line_num, token);
                    fclose(fp_src);
                    fclose(fp_obj);
                    return false;
                }
				
				// verifica se operand é número literal ou label
				if (isdigit((unsigned char)operand[0]) || (operand[0] == '-' && isdigit((unsigned char)operand[1]))) {
					op1 = atoi(operand);
                } else {
					int addr = get_symbol_address(operand);
                    if (addr < 0) {
						fprintf(stderr, "Linha %d: símbolo \"%s\" indefinido.\n", line_num, operand);
                        fclose(fp_src);
                        fclose(fp_obj);
                        return false;
                    }
                    op1 = addr;
                }
				// mover ptr após primeiro operando
				ptr += strlen(operand);
                trim(ptr);
            }
			if (expected_operands >= 2) {
				char operand[MAX_TOKEN_LEN];
				if (sscanf(ptr, "%s", operand) != 1) {
                    fprintf(stderr, "Linha %d: faltou operando 2 em \"%s\".\n", line_num, token);
                    fclose(fp_src);
                    fclose(fp_obj);
                    return false;
                }
				if (isdigit((unsigned char)operand[0]) ||
				    (operand[0] == '-' &&
				     isdigit((unsigned char)operand[1]))) {
					op2 = atoi(operand);
                } else {
					int addr = get_symbol_address(operand);
                    if (addr < 0) {
						fprintf(stderr, "Linha %d: símbolo \"%s\" indefinido.\n", line_num, operand);
                        fclose(fp_src);
                        fclose(fp_obj);
                        return false;
                    }
                    op2 = addr;
                }
				ptr += strlen(operand);
                trim(ptr);
            }
			if (expected_operands >= 3) {
				char operand[MAX_TOKEN_LEN];
				if (sscanf(ptr, "%s", operand) != 1) {
                    fprintf(stderr, "Linha %d: faltou operando 3 em \"%s\".\n", line_num, token);
                    fclose(fp_src);
                    fclose(fp_obj);
                    return false;
                }
				if (isdigit((unsigned char)operand[0]) ||
				    (operand[0] == '-' &&
				     isdigit((unsigned char)operand[1]))) {
					op3 = atoi(operand);
                } else {
					int addr = get_symbol_address(operand);
                    if (addr < 0) {
						fprintf(stderr, "Linha %d: símbolo \"%s\" indefinido.\n", line_num, operand);
                        fclose(fp_src);
                        fclose(fp_obj);
                        return false;
                    }
                    op3 = addr;
                }
				ptr += strlen(operand);
                trim(ptr);
            }

			// Escreve no arquivo objeto: “opcode op1 op2 op3”
			// Se instrução tiver <3 operandos, os slots de operando “não usados” serão "-1".
			fprintf(
				fp_obj, 
				"%d %d %d %d\n",
				instr_table[opc].opcode, op1, op2, op3
			);
            loc++;
        }
    }

    fclose(fp_src);
    fclose(fp_obj);
    return true;
}

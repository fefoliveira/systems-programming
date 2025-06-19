// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "assembler.h"
#include "../utils/utils.h"

/* --------------------------------------------------------------------------
	Definição estática da tabela de instruções (mnem → opcode, nº operandos)
   - ADD, SUB, MUL, DIV → 3 operandos (3 registradores)
   - MV, ST         	→ 2 operandos (1 registrador + 1 endereço)
   - JMP           		→ 1 operando  (1 endereço)
   - JEQ, JGT, JLT  	→ 3 operandos (2 registradores + 1 endereço)
   - W, R           	→ 1 operando  (1 endereço)
   - STP            	→ 0 operandos
   -------------------------------------------------------------------------- */
static const InstrDef instr_table[] = {
	{ "ADD", 0, 3 },  
	{ "SUB", 1, 3 }, 
	{ "MUL", 2, 3 }, 
	{ "DIV", 3, 3 },
	{ "MV", 4, 2 },	  
	{ "ST", 5, 2 },  
	{ "JMP", 6, 1 }, 
	{ "JEQ", 7, 3 },
	{ "JGT", 8, 3 },  
	{ "JLT", 9, 3 }, 
	{ "W", 10, 1 },  
	{ "R", 11, 1 },
	{ "STP", 12, 0 },
};

static const int NUM_INSTR = sizeof(instr_table) / sizeof(InstrDef);

/* --------------------------------------------------------------------------
   Estrutura da tabela de símbolos
   -------------------------------------------------------------------------- */
static Symbol symbol_table[MAX_SYMBOLS];
static int symbol_count = 0;

/* --------------------------------------------------------------------------
   Contador de localização (LOCCTR: Location Counter): "endereço corrente" 
   durante montagem
   -------------------------------------------------------------------------- */
static int LOCCTR = 0;

/* --------------------------------------------------------------------------
   Verifica se uma string é um pseudo-mnemônico de uma pseudo-instrução 
   (SPACE, CONST ou END), preenchendo outType com o tipo correspondente.
   Retorna true se for pseudo-instr, false caso contrário.
   -------------------------------------------------------------------------- */
bool is_pseudo(const char *token, PseudoType *outType)
{
	if (strcmp(token, "SPACE") == 0) {
		*outType = PSEUDO_SPACE;
		return true;
	} else if (strcmp(token, "CONST") == 0) {
		*outType = PSEUDO_CONST;
		return true;
	} else if (strcmp(token, "END") == 0) {
		*outType = PSEUDO_END;
		return true;
	}
	*outType = PSEUDO_NONE;
	return false;
}

/* --------------------------------------------------------------------------
   Procura um mnemônico na tabela de instruções. Se encontrar, retorna o opcode.
   Se não, retorna -1.
   -------------------------------------------------------------------------- */
int lookup_opcode(const char *mnemonic)
{
	for (int i = 0; i < NUM_INSTR; i++) {
		if (strcmp(instr_table[i].mnemonic, mnemonic) == 0) {
			return instr_table[i].opcode;
		}
	}
	return -1;
}

/* --------------------------------------------------------------------------
   Insere novo símbolo (label) na tabela de símbolos, com dado endereço.
   Se já existir (redefinido), imprime erro e retorna false.
   -------------------------------------------------------------------------- */
static bool add_symbol(const char *label, int address)
{
	for (int i = 0; i < symbol_count; i++) {
		if (strcmp(symbol_table[i].label, label) == 0) {
			fprintf(stderr, "Erro: símbolo \"%s\" redefinido.\n", label);
			return false;
		}
	}
	if (symbol_count >= MAX_SYMBOLS) {
		fprintf(stderr, "Erro: tabela de símbolos cheia.\n");
		return false;
	}
	strncpy(symbol_table[symbol_count].label, label, MAX_LABEL_LEN);
	symbol_table[symbol_count].label[MAX_LABEL_LEN - 1] = '\0';
	symbol_table[symbol_count].address = address;
	symbol_count++;
	return true;	
}

/* --------------------------------------------------------------------------
   Procura um símbolo na tabela e retorna seu endereço. Se não encontrar, retorna -1.
   -------------------------------------------------------------------------- */
static int get_symbol_address(const char *label)
{
	for (int i = 0; i < symbol_count; i++) {
		if (strcmp(symbol_table[i].label, label) == 0) {
			return symbol_table[i].address;
		}
	}
	return -1;
}

/* --------------------------------------------------------------------------
   Imprime a tabela de símbolos (para debug)
   -------------------------------------------------------------------------- */
void print_symbol_table(void)
{
	printf("\n>>> Tabela de Símbolos (label -> endereço):\n");
	for (int i = 0; i < symbol_count; i++) {
		printf("  %s -> %d\n", symbol_table[i].label,
		       symbol_table[i].address);
	}
	printf("\n");
}

/* --------------------------------------------------------------------------
   PASSAGEM 1: 
   - Abre o arquivo de fonte (.txt)
   - Percorre linha a linha, identifica labels e mnemônicos
   - Atualiza LOCCTR conforme instruções ou pseudo-instruções
   - Armazena labels e seus endereços na tabela de símbolos
   Se encontrar erro (símbolo redefinido, instrução inválida), retorna false.
   -------------------------------------------------------------------------- */
bool first_pass(const char *src_filename)
{
	FILE *fp = fopen(src_filename, "r");
	if (!fp) {
		perror("Erro ao abrir arquivo fonte");
		return false;
	}

	char line[MAX_LINE_LEN];
	LOCCTR = 0;
	symbol_count = 0;

	int line_num = 0;
	while (fgets(line, sizeof(line), fp) != NULL) {
		line_num++;
		char *ptr = line; // ex.: "loop: ADD R1 R2 R3"
		trim(ptr);
		if (ptr[0] == '\0' || ptr[0] == '#') {
			continue; // linha vazia ou comentário
		}

		// Separar possível label no início (ex: "loop:")
		char label[MAX_LABEL_LEN] = { 0 };
		if (strchr(ptr, ':') != NULL) {
			// se há ‘:’ → extrai até os dois-pontos
			char *colon = strchr(ptr, ':');
			int len = colon - ptr;
			if (len >= MAX_LABEL_LEN) {
				len = MAX_LABEL_LEN - 1;
			}
			strncpy(label, ptr, len);
			label[len] = '\0';
			trim(label);

			// adiciona à tabela de símbolos com endereço LOCCTR atual
			bool ok = add_symbol(label, LOCCTR);
			if (!ok) {
				fclose(fp);
				return false; // símbolo redefinido ou tabela cheia
			}

			// avança ptr para depois do “:”
			ptr = colon + 1;	// ex.: " ADD R1 R2 R3"
			trim(ptr);	// ex.: "ADD R1 R2 R3"
		}

		if (ptr[0] == '\0') {
			// linha tinha só label e nada mais; conta 0 palavras de máquina
			continue;
		}

		// Agora ptr aponta para o mnemônico ou pseudo-mnemônico
		// Extrai o primeiro token (separa por espaço/tab)
		char token[MAX_TOKEN_LEN] = { 0 };
		int read = sscanf(ptr, "%s", token);
		if (read != 1) {
			fprintf(stderr, "Linha %d: formato inválido.\n",
				line_num);
			fclose(fp);
			return false;
		}

		// Verifica se o token é pseudo-instr (SPACE, CONST, END)
		PseudoType pseudo;
		if (is_pseudo(token, &pseudo)) {
			switch (pseudo) {
				case PSEUDO_SPACE: {
					// "SPACE N"
					int n;
					bool ok = (sscanf(ptr + strlen(token), "%d", &n) == 1 && n >= 0);
					if (!ok) {
						fprintf(stderr, "Linha %d: SPACE exige um inteiro >= 0.\n", line_num);
						fclose(fp);
						return false;
					}
					LOCCTR += n; // aloca N palavras
					break;
				}
				case PSEUDO_CONST: {
					// "CONST K"
					int k;
					bool ok = (sscanf(ptr + strlen(token), "%d", &k) == 1);
					if (!ok) {
						fprintf(stderr, "Linha %d: CONST exige um valor inteiro.\n", line_num);
						fclose(fp);
						return false;
					}
					LOCCTR += 1; // uma palavra (armazena a constante k)
					break;
				}
				case PSEUDO_END: {
					// não ocupa espaço; fim do código. A leitura é interrompida.
					fclose(fp);
					return true;
				}
				default:
					break;
			}
		} else {
			// Não é pseudo: deve ser instrução normal
			int opc = lookup_opcode(token);
			if (opc < 0) {
				fprintf(stderr, "Linha %d: instrução \"%s\" inválida.\n", line_num, token);
				fclose(fp);
				return false;
			}
			LOCCTR += 1; // conta 1 palavra para a instrução
		}
	}

	fclose(fp);
	return true;
}

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
			switch (pseudo) {
				case PSEUDO_SPACE: {
					int n;
					sscanf(ptr + strlen(token), "%d", &n);
					for (int i = 0; i < n; i++) {
						fprintf(fp_obj, "0\n"); // SPACE: gera 'n' linhas com zeros
						loc++;
					}
					break;
				}
				case PSEUDO_CONST: {
					int k;
					sscanf(ptr + strlen(token), "%d", &k);
					fprintf(fp_obj, "%d\n", k); // CONST: gera uma linha com o valor 'k'
					loc++;
					break;
				}
				case PSEUDO_END: {
					// fim; montagem encerrada. Não gera-se mais nada.
					fclose(fp_src);
					fclose(fp_obj);
					return true;
				}
				default:
					break;
			}
		} else {
			// É uma instrução normal
			int opc = lookup_opcode(token);
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

/* --------------------------------------------------------------------------
   main: chamada do montador em linha de comando
   Uso: assembler <arquivo-fonte.txt> <arquivo-objeto.txt>
   -------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Uso correto: %s <arquivo-fonte.txt> <arquivo-objeto.txt>\n", argv[0]);
		return EXIT_FAILURE;
	}
	const char *src_file = argv[1];
	const char *obj_file = argv[2];

	// 1ª passagem: constrói tabela de símbolos
	if (!first_pass(src_file)) {
		fprintf(stderr, "Erro na primeira passagem. Abortando.\n");
		return EXIT_FAILURE;
	}

	print_symbol_table();

	// 2ª passagem: gera arquivo objeto
	if (!second_pass(src_file, obj_file)) {
		fprintf(stderr, "Erro na segunda passagem. Abortando.\n");
		return EXIT_FAILURE;
	}

	printf("Montagem concluída: \"%s\" → \"%s\"\n", src_file, obj_file);
	return EXIT_SUCCESS;
}

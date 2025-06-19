// clang-format off
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "assembler.h"

/* --------------------------------------------------------------------------
	Definição global da tabela de instruções (mnem → opcode, nº operandos)
   - ADD, SUB, MUL, DIV → 3 operandos (3 registradores)
   - MV, ST         	→ 2 operandos (1 registrador + 1 endereço)
   - JMP           		→ 1 operando  (1 endereço)
   - JEQ, JGT, JLT  	→ 3 operandos (2 registradores + 1 endereço)
   - W, R           	→ 1 operando  (1 endereço)
   - STP            	→ 0 operandos
   -------------------------------------------------------------------------- */
const InstrDef instr_table[] = {
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

const int NUM_INSTR = sizeof(instr_table) / sizeof(InstrDef);

/* --------------------------------------------------------------------------
   Estrutura da tabela de símbolos
   -------------------------------------------------------------------------- */
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

/* --------------------------------------------------------------------------
   Contador de localização (LOCCTR: Location Counter): "endereço corrente" 
   durante montagem
   -------------------------------------------------------------------------- */
int LOCCTR = 0;

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
bool add_symbol(const char *label, int address)
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
int get_symbol_address(const char *label)
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
		printf("  • %s -> %d\n", symbol_table[i].label, symbol_table[i].address);
	}
	printf("\n");
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
	bool ok = first_pass(src_file);
	if (!ok) {
		fprintf(stderr, "Erro na primeira passagem. Abortando.\n");
		return EXIT_FAILURE;
	}

	print_symbol_table();

	// 2ª passagem: gera arquivo objeto
	ok = second_pass(src_file, obj_file);
	if (!ok) {
		fprintf(stderr, "Erro na segunda passagem. Abortando.\n");
		return EXIT_FAILURE;
	}

	printf("Montagem concluída: \"%s\" → \"%s\"\n", src_file, obj_file);
	return EXIT_SUCCESS;
}

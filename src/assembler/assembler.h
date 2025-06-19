// src/assembler/assembler.h

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

#define MAX_LINE_LEN 256
#define MAX_LABEL_LEN 32
#define MAX_TOKEN_LEN 32
#define MAX_SYMBOLS 256 // número máximo de labels
#define MAX_INSTRUCTIONS 256 // instruções diferentes (ADD, SUB, etc.)

// Pseudo-instruções reconhecidas
typedef enum {
	PSEUDO_SPACE,
	PSEUDO_CONST,
	PSEUDO_END,
	PSEUDO_NONE // não é pseudo-instr
} PseudoType;

// Formato de instrução (tabela de instruções)
typedef struct {
	char mnemonic[MAX_TOKEN_LEN];
	int opcode;
	int operand_count;
} InstrDef;

// Tabela de símbolos
typedef struct {
	char label[MAX_LABEL_LEN];
	int address;
} Symbol;

bool first_pass(const char *src_filename);
bool second_pass(const char *src_filename, const char *obj_filename);

void print_symbol_table(void);
int lookup_opcode(const char *mnemonic);
bool is_pseudo(const char *token, PseudoType *outType);

#endif

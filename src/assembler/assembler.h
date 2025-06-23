// clang-format off

#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>

#define MAX_LINE_LEN 256
#define MAX_LABEL_LEN 32
#define MAX_TOKEN_LEN 32
#define MAX_SYMBOLS 256 // número máximo de labels por módulo/local
#define MAX_INSTR 256 	// instruções diferentes (ADD, SUB, etc.)

// Para múltiplos módulos:
#define MAX_MODULES 16 // limite de módulos para linkagem
#define MAX_EXTAB (MAX_MODULES * MAX_SYMBOLS)

// Pseudo-instruções reconhecidas
typedef enum {
	PSEUDO_SPACE,
	PSEUDO_CONST,
	PSEUDO_END,
	PSEUDO_NONE // não é pseudo-instr
} PseudoType;

// Definição de instrução (tabela de instruções)
typedef struct {
	char mnemonic[MAX_TOKEN_LEN];
	int opcode;
	int operand_count;
} InstrDef;

// Tabela de símbolos (modo single-module)
typedef struct {
	char label[MAX_LABEL_LEN];
	int address; // endereço absoluto no módulo único
} Symbol;

// Informação de cada módulo quando em multi-module
typedef struct {
	const char *filename; // ponteiro para nome do arquivo-fonte
	int size; // LOCCTR relativo (número de palavras)
	int CSADDR; // base atribuída no endereço final

	// definições de símbolos locais:
	int def_count;
	struct {
		char label[MAX_LABEL_LEN];
		int rel_addr; // endereço relativo dentro do módulo
		bool is_global; // marcado se for um símbolo GLOBAL
	} defs[MAX_SYMBOLS];

	// símbolos externos referenciados:
	int extern_count;
	char externs[MAX_SYMBOLS][MAX_LABEL_LEN];
} ModuleInfo;

// Tabela de símbolos externa (EXTAB) para MULTI-MODULE
typedef struct {
    char symbol[MAX_LABEL_LEN];
    int address;  // endereço absoluto final
} EXTABEntry;


// Variáveis para single-module
extern const InstrDef instr_table[];
extern const int NUM_INSTR;
extern Symbol symbol_table[];
extern int symbol_count;
extern int instr_LOCCTR;
extern int data_LOCCTR;

// Variáveis para multi-module
extern ModuleInfo modules[];
extern int module_count;

// Funções de passagem single-module
bool first_pass(const char *src_filename);
bool second_pass(const char *src_filename, const char *obj_filename);

// Funções de passagem multi-module
bool first_pass_multi(int filecount, const char *filenames[]);
bool second_pass_multi(int filecount, const char *filenames[], const char *out_filename);

// Utilitários single-module
bool is_pseudo(const char *token, PseudoType *out);
int lookup_opcode(const char *mnemonic);
int get_symbol_address(const char *label);
bool add_symbol(const char *label, int address);
void print_symbol_table(void);

// Utilitários multi-module
int EXTAB_lookup(const char *symbol);
bool EXTAB_add(const char *symbol, int address);
void print_EXTAB(void);

#endif // ASSEMBLER_H

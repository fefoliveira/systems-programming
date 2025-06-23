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
    { "ADD",  0, 3 },
    { "SUB",  1, 3 },
    { "MUL",  2, 3 },
    { "DIV", 	3, 3 },
    { "MV",   4, 2 },
    { "ST",   5, 2 },
    { "JMP",  6, 1 },
    { "JEQ",  7, 3 },
    { "JGT",  8, 3 },
    { "JLT",  9, 3 },
    { "W",   10, 1 },
    { "R",   11, 1 },
    { "STP", 12, 0 },
};
const int NUM_INSTR = sizeof(instr_table) / sizeof(InstrDef);

/* --------------------------------------------------------------------------
   Estrutura da tabela de símbolos (SINGLE-MODULE)
   -------------------------------------------------------------------------- */
Symbol symbol_table[MAX_SYMBOLS];
int symbol_count = 0;

/* --------------------------------------------------------------------------
   Contador de localização (LOCCTR: Location Counter): "endereço corrente" 
   durante montagem (SINGLE-MODULE).
   -------------------------------------------------------------------------- */
int LOCCTR = 0;

/* --------------------------------------------------------------------------
   Multi-módulo: array de módulos e contagem.
   -------------------------------------------------------------------------- */
ModuleInfo modules[MAX_MODULES];
int module_count = 0;

/* --------------------------------------------------------------------------
   Multi-módulo: estrutura da tabela externa de símbolos (EXTAB).
   -------------------------------------------------------------------------- */
EXTABEntry EXTAB[MAX_EXTAB];
int EXTAB_count = 0;

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
   Se já existir (redefinido), imprime erro e retorna false. (SINGLE-MODULE)
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
   Procura um símbolo na tabela e retorna seu endereço. 
   Se não encontrar, retorna -1. (SINGLE-MODULE)
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
   Imprime a tabela de símbolos. (SINGLE-MODULE)
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
   Busca na EXTAB. (MULTI-MODULE)
   -------------------------------------------------------------------------- */
int EXTAB_lookup(const char *symbol)
{
    for (int i = 0; i < EXTAB_count; i++) {
        if (strcmp(EXTAB[i].symbol, symbol) == 0) {
            return EXTAB[i].address;
        }
    }
    return -1;
}

/* --------------------------------------------------------------------------
   Insere novo símbolo (label) na EXTAB e verifica duplicação. (MULTI-MODULE)
   -------------------------------------------------------------------------- */
bool EXTAB_add(const char *symbol, int address)
{
    for (int i = 0; i < EXTAB_count; i++) {
        if (strcmp(EXTAB[i].symbol, symbol) == 0) {
            // já existe: redefinição global?
            fprintf(stderr, "Erro: símbolo global \"%s\" definido em múltiplos módulos.\n", symbol);
            return false;
        }
    }
    if (EXTAB_count >= MAX_EXTAB) {
        fprintf(stderr, "Erro: EXTAB cheia.\n");
        return false;
    }
    strncpy(EXTAB[EXTAB_count].symbol, symbol, MAX_LABEL_LEN);
    EXTAB[EXTAB_count].symbol[MAX_LABEL_LEN - 1] = '\0';
    EXTAB[EXTAB_count].address = address;
    EXTAB_count++;
    return true;
}

void print_EXTAB(void)
{
	printf("\n>>> EXTAB (símbolos globais e seus endereços):\n");
	for (int i = 0; i < EXTAB_count; i++) {
		printf("  • %s -> %d\n", EXTAB[i].symbol, EXTAB[i].address);
	}
	printf("\n");
}

/* --------------------------------------------------------------------------
   MAIN: chamada do montador: detecta single ou multi-module
   -------------------------------------------------------------------------- */
int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Uso:\n");
        fprintf(stderr, "  Para montar um único arquivo fonte: %s <arquivo-fonte.txt> <arquivo-objeto.txt>\n", argv[0]);
        fprintf(stderr, "  Para linkar e montar múltiplos: %s <mod1.txt> <mod2.txt> ... -o <saida.obj>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Verifica se há a opção "-o"
    int out_i = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            out_i = i;
            break;
        }
    }

    if (out_i < 0) {
        // Modo single-module: deve ser exatamente 3 argumentos: argv[1]=source, argv[2]=dest
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
    } else {
        // Modo multi-module: argumentos antes de "-o" são módulos
        int nmods = out_i - 1;
        if (nmods < 1) {
            fprintf(stderr, "Erro: especifique ao menos um módulo antes de '-o'.\n");
            return EXIT_FAILURE;
        }
        if (out_i + 1 >= argc) {
            fprintf(stderr, "Erro: falta nome de saída após '-o'.\n");
            return EXIT_FAILURE;
        }
        const char **mods = (const char **)&argv[1];
        const char *out_fname = argv[out_i + 1];

        //1ª passagem multi: constrói EXTAB e módulos
        bool ok = first_pass_multi(nmods, mods);
		if (!ok) {
            fprintf(stderr, "Erro na primeira passagem multi-módulo. Abortando.\n");
            return EXIT_FAILURE;
        }
        
		print_EXTAB();

        // 2ª passagem multi: junta os módulos e gera o arquivo objeto
        ok = second_pass_multi(nmods, mods, out_fname);
		if (!ok) {
            fprintf(stderr, "Erro na segunda passagem multi-módulo. Abortando.\n");
            return EXIT_FAILURE;
        }
        printf("Linkagem e montagem concluídas: \"%s\"\n", out_fname);
        return EXIT_SUCCESS;
    }
}

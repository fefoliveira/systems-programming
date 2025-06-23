// clang-format off

#include <stdio.h>
#include <string.h>
#include "../utils/utils.h"
#include "assembler.h"

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

		// Remover comentários (se houver)
        char *comment = strchr(ptr, '#');
        if (comment) {
				*comment = '\0'; // corta a linha no comentário
            trim(ptr);
            if (ptr[0] == '\0') {
					continue; // linha só tinha comentário
            }
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
            fprintf(stderr, "Linha %d: formato inválido.\n", line_num);
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
                    char *p = ptr + strlen(token);
                    trim(p);
					bool ok = (sscanf(p, "%d", &n) == 1 && n >= 0);
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
                    char *p = ptr + strlen(token);
                    trim(p);
					bool ok = (sscanf(p, "%d", &k) == 1);
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
    } // while fgets

    fclose(fp);
    return true;
}

// clang-format off

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../utils/utils.h"
#include "../assembler.h"

// Buffer de linha
static char line_buffer[MAX_LINE_LEN];

// ---------------------------------------------------------------------------
// MULTI-MÓDULO: segunda passagem que gera o objeto final unidos
// ---------------------------------------------------------------------------
bool second_pass_multi(int filecount, const char *filenames[], const char *out_filename)
{
    FILE *fp_out = fopen(out_filename, "w");
    if (!fp_out) {
        perror("fopen saída");
        return false;
    }

    // Para cada módulo, reabrir e escrever instruções ajustadas
    for (int i = 0; i < filecount; i++) {
        ModuleInfo *mod = &modules[i];
        FILE *fp = fopen(mod->filename, "r");
        if (!fp) {
            perror("fopen módulo");
            fclose(fp_out);
            return false;
        }
        int line_num = 0;
        while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
            line_num++;
            char *ptr = line_buffer;
            trim(ptr);
            if (ptr[0] == '\0' || ptr[0] == '#') {
                continue;
            }
            // Remover comentário inline
            char *comment = strchr(ptr, '#');
            if (comment) {
                *comment = '\0';
                trim(ptr);
                if (ptr[0] == '\0') continue;
            }
            // Ignorar diretivas GLOBAL/EXTERN
            char firsttok[MAX_TOKEN_LEN] = {0};
            if (sscanf(ptr, "%s", firsttok) == 1) {
                if (strcmp(firsttok, "GLOBAL") == 0 || strcmp(firsttok, "EXTERN") == 0) {
                    continue;
                }
            }
            // Pular label
            if (strchr(ptr, ':') != NULL) {
                char *colon = strchr(ptr, ':');
                ptr = colon + 1;
                trim(ptr);
                if (ptr[0] == '\0') continue;
            }
            // Extrair token
            char token[MAX_TOKEN_LEN] = {0};
            if (sscanf(ptr, "%s", token) != 1) {
                continue;
            }
            // Pseudo?
            PseudoType pseudo;
            if (is_pseudo(token, &pseudo)) {
                // Se for pseudo-instrução, só checa os erros e dá continue
                if (pseudo == PSEUDO_SPACE) {
                    int n;
                    char *p = ptr + strlen(token);
                    trim(p);
                    if (sscanf(p, "%d", &n) != 1 || n < 0) {
                        fprintf(stderr, "Erro %s: SPACE exige inteiro >= 0.\n", mod->filename);
                        fclose(fp);
                        fclose(fp_out);
                        return false;
                    }
                } else if (pseudo == PSEUDO_CONST) {
                    int k;
                    char *p = ptr + strlen(token);
                    trim(p);
                    if (sscanf(p, "%d", &k) != 1) {
                        fprintf(stderr, "Erro %s: CONST exige valor inteiro.\n", mod->filename);
                        fclose(fp);
                        fclose(fp_out);
                        return false;
                    }
                } else if (pseudo == PSEUDO_END) {
                    break;
                }
                continue;
            } else {
                // Instrução normal
                int opc = lookup_opcode(token);
                if (opc < 0) {
                    fprintf(stderr, "Erro %s: instrução \"%s\" inválida.\n", mod->filename, token);
                    fclose(fp);
                    fclose(fp_out);
                    return false;
                }
                ptr += strlen(token);
                trim(ptr);
                int ops[3] = { -1, -1, -1 };
                int nargs = instr_table[opc].operand_count;
                for (int j = 0; j < nargs; j++) {
                    char oper[MAX_TOKEN_LEN];
                    if (sscanf(ptr, "%s", oper) != 1) {
                        fprintf(stderr, "Erro %s: faltou operando %d em \"%s\".\n",
                                mod->filename, j + 1, token);
                        fclose(fp);
                        fclose(fp_out);
                        return false;
                    }
                    if (isdigit((unsigned char)oper[0]) ||
                        (oper[0] == '-' && isdigit((unsigned char)oper[1]))) {
                        ops[j] = atoi(oper);
                    } else {
                        // buscar em EXTAB (symbols globais e locais já registrados)
                        int abs_addr = EXTAB_lookup(oper);
                        if (abs_addr < 0) {
                            fprintf(stderr, "Erro %s: símbolo \"%s\" não definido.\n",
                                    mod->filename, oper);
                            fclose(fp);
                            fclose(fp_out);
                            return false;
                        }
                        ops[j] = abs_addr;
                    }
                    ptr += strlen(oper);
                    trim(ptr);
                }
                fprintf(fp_out, "%d %d %d %d\n", instr_table[opc].opcode, ops[0], ops[1], ops[2]);
            }
        }
        fclose(fp);
    }
    fclose(fp_out);
    return true;
}

// clang-format off

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../utils/utils.h"
#include "../assembler.h"

// Buffer de linha
static char line_buffer[MAX_LINE_LEN];

// Processa um único módulo (primeira passagem local):
// Preenche ModuleInfo: defs (com rel_addr), externs, e size.
static bool process_module_first(const char *filename, ModuleInfo *mod)
{
    FILE *fp = fopen(filename, "r");
    if (!fp) {
        perror("fopen módulo");
        return false;
    }
    mod->filename = filename;
    mod->size = 0;
    mod->def_count = 0;
    mod->extern_count = 0;

    int LOCCTR_local = 0;
    int line_num = 0;
    while (fgets(line_buffer, sizeof(line_buffer), fp) != NULL) {
        line_num++;
        char *ptr = line_buffer;
        trim(ptr);
        if (ptr[0] == '\0' || ptr[0] == '#') {
            continue;
        }
        // Remover comentário inline
        char *cmt = strchr(ptr, '#');
        if (cmt) {
            *cmt = '\0';
            trim(ptr);
            if (ptr[0] == '\0') continue;
        }
        // Extrair primeiro token para verificar diretivas
        char token[MAX_TOKEN_LEN] = {0};
        if (sscanf(ptr, "%s", token) == 1) {
            if (strcmp(token, "GLOBAL") == 0) {
                // Diretiva GLOBAL <símbolo>
                char sym[MAX_TOKEN_LEN] = {0};
                char *p = ptr + strlen(token);
                trim(p);
                if (sscanf(p, "%s", sym) == 1) {
                    // Verifica se já existe em defs
                    int found = -1;
                    for (int i = 0; i < mod->def_count; i++) {
                        if (strcmp(mod->defs[i].label, sym) == 0) {
                            found = i;
                            break;
                        }
                    }
                    if (found < 0) {
                        // adiciona entrada com rel_addr=-1 (será preenchido quando achar label:)
                        if (mod->def_count < MAX_SYMBOLS) {
                            strncpy(mod->defs[mod->def_count].label, sym, MAX_LABEL_LEN);
                            mod->defs[mod->def_count].label[MAX_LABEL_LEN - 1] = '\0';
                            mod->defs[mod->def_count].rel_addr = -1;
                            mod->defs[mod->def_count].is_global = true;
                            mod->def_count++;
                        }
                    } else {
                        // já havia entrado, apenas marca global
                        mod->defs[found].is_global = true;
                    }
                }
                continue;
            }
            if (strcmp(token, "EXTERN") == 0) {
                // Diretiva EXTERN <símbolo>
                char sym[MAX_TOKEN_LEN] = {0};
                char *p = ptr + strlen(token);
                trim(p);
                if (sscanf(p, "%s", sym) == 1) {
                    // registrar em externs, sem duplicar
                    bool dup = false;
                    for (int i = 0; i < mod->extern_count; i++) {
                        if (strcmp(mod->externs[i], sym) == 0) {
                            dup = true;
                            break;
                        }
                    }
                    if (!dup && mod->extern_count < MAX_SYMBOLS) {
                        strncpy(mod->externs[mod->extern_count], sym, MAX_LABEL_LEN);
                        mod->externs[mod->extern_count][MAX_LABEL_LEN - 1] = '\0';
                        mod->extern_count++;
                    }
                }
                continue;
            }
        }
        // Verifica label
        if (strchr(ptr, ':') != NULL) {
            char *colon = strchr(ptr, ':');
            int len = colon - ptr;
            if (len >= MAX_LABEL_LEN) len = MAX_LABEL_LEN - 1;
            char label[MAX_LABEL_LEN] = {0};
            strncpy(label, ptr, len);
            label[len] = '\0';
            trim(label);
            // registrar definição local: endereço relativo = LOCCTR_local
            int found = -1;
            for (int i = 0; i < mod->def_count; i++) {
                if (strcmp(mod->defs[i].label, label) == 0) {
                    found = i;
                    break;
                }
            }
            if (found < 0) {
                // nova definição local, is_global=false
                if (mod->def_count < MAX_SYMBOLS) {
                    strncpy(mod->defs[mod->def_count].label, label, MAX_LABEL_LEN);
                    mod->defs[mod->def_count].label[MAX_LABEL_LEN - 1] = '\0';
                    mod->defs[mod->def_count].rel_addr = LOCCTR_local;
                    mod->defs[mod->def_count].is_global = false;
                    mod->def_count++;
                }
            } else {
                // já existia entrada (talvez marcada GLOBAL anteriormente), preencher rel_addr
                mod->defs[found].rel_addr = LOCCTR_local;
            }
            // avança ptr após ":"
            ptr = colon + 1;
            trim(ptr);
            if (ptr[0] == '\0') {
                continue;
            }
        }
        // Agora ptr aponta para mnemônico ou pseudo
        char token2[MAX_TOKEN_LEN] = {0};
        if (sscanf(ptr, "%s", token2) != 1) {
            continue;
        }
        // Pseudo?
        PseudoType pseudo;
        if (is_pseudo(token2, &pseudo)) {
            if (pseudo == PSEUDO_SPACE) {
                int n;
                char *p = ptr + strlen(token2);
                trim(p);
                if (sscanf(p, "%d", &n) != 1 || n < 0) {
                    fprintf(stderr, "Erro %s: SPACE exige inteiro >= 0.\n", filename);
                    fclose(fp);
                    return false;
                }
                LOCCTR_local += n;
            } else if (pseudo == PSEUDO_CONST) {
                LOCCTR_local += 1;
            } else if (pseudo == PSEUDO_END) {
                break;
            }
        } else {
            // Instrução normal
            int opc = lookup_opcode(token2);
            if (opc < 0) {
                fprintf(stderr, "Erro %s: instrução \"%s\" inválida.\n", filename, token2);
                fclose(fp);
                return false;
            }
            LOCCTR_local += 1;
        }
    }
    mod->size = LOCCTR_local;
    fclose(fp);
    return true;
}

// Primeira passagem multi-módulo
bool first_pass_multi(int filecount, const char *filenames[])
{
    module_count = 0;
    // Processar cada módulo para obter defs, externs e tamanho
    for (int i = 0; i < filecount; i++) {
        if (module_count >= MAX_MODULES) {
            fprintf(stderr, "Erro: excesso de módulos (limite = %d).\n", MAX_MODULES);
            return false;
        }
        if (!process_module_first(filenames[i], &modules[module_count])) {
            return false;
        }
        module_count++;
    }
    // Construir EXTAB e atribuir CSADDR
    int cur_base = 0;
    // Reiniciar EXTAB
    extern int EXTAB_count;  // definido em assembler.c
    extern EXTABEntry EXTAB[]; // conforme em assembler.c
    EXTAB_count = 0;
    for (int i = 0; i < module_count; i++) {
        modules[i].CSADDR = cur_base;
        // Para cada definição local marcada is_global, adicionar a EXTAB
        for (int j = 0; j < modules[i].def_count; j++) {
            if (modules[i].defs[j].is_global) {
                const char *sym = modules[i].defs[j].label;
                int rel = modules[i].defs[j].rel_addr;
                if (rel < 0) {
                    fprintf(stderr, "Erro: símbolo GLOBAL \"%s\" em %s não definido.\n",
                            sym, modules[i].filename);
                    return false;
                }
                int abs_addr = cur_base + rel;
                if (!EXTAB_add(sym, abs_addr)) {
                    return false;
                }
            }
        }
        cur_base += modules[i].size;
    }
    // Verificar externos: cada EXTERN deve estar em EXTAB
    for (int i = 0; i < module_count; i++) {
        for (int j = 0; j < modules[i].extern_count; j++) {
            const char *sym = modules[i].externs[j];
            if (EXTAB_lookup(sym) < 0) {
                fprintf(stderr, "Erro: símbolo EXTERN \"%s\" em %s não resolvido.\n",
                        sym, modules[i].filename);
                return false;
            }
        }
    }
    return true;
}

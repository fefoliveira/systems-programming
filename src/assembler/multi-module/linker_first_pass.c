// clang-format off

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../../utils/utils.h"
#include "../assembler.h"

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
    mod->instr_size = 0;
    mod->data_size = 0;
    mod->def_count = 0;
    mod->extern_count = 0;

    char line[MAX_LINE_LEN];
    int instr_LOCCTR_local = 0;
    int data_LOCCTR_local = 0;
    int line_num = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        line_num++;
        char *ptr = line;
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
                        // adiciona entrada com rel_addr=-1 (será preenchido quando achar o "label:" posterior)
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
        int label_idx = -1;
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
                    mod->defs[mod->def_count].rel_addr = -1; // será preenchido assim que achar o mnemônico
                    mod->defs[mod->def_count].is_global = false;
                    label_idx = mod->def_count;
                    mod->def_count++;
                }
            } else {
                // já existia entrada (talvez marcada GLOBAL anteriormente), preencher "rel_addr" na posição "found"
                label_idx = found;
            }
            // avança ptr após ":"
            ptr = colon + 1;
            trim(ptr);
            if (ptr[0] == '\0') {
                continue; // linha tinha só label e nada mais
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
            if (pseudo != PSEUDO_END && label_idx >= 0) {
                mod->defs[label_idx].rel_addr = data_LOCCTR_local;
                mod->defs[label_idx].is_data = true; // marca como definição de dado
            }
            if (pseudo == PSEUDO_SPACE) {
                int n;
                char *p = ptr + strlen(token2);
                trim(p);
                if (sscanf(p, "%d", &n) != 1 || n < 0) {
                    fprintf(stderr, "Erro %s: SPACE exige inteiro >= 0.\n", filename);
                    fclose(fp);
                    return false;
                }
                data_LOCCTR_local += n;
            } else if (pseudo == PSEUDO_CONST) {
                data_LOCCTR_local += 1;
            } else if (pseudo == PSEUDO_END) {
                break;
            }
        } else {
            // Instrução normal
            if (label_idx >= 0) {
                mod->defs[label_idx].rel_addr = instr_LOCCTR_local;
                mod->defs[label_idx].is_data = false; // marca como definição de instrução
            }
            int opc = lookup_opcode(token2);
            if (opc < 0) {
                fprintf(stderr, "Erro %s: instrução \"%s\" inválida.\n", filename, token2);
                fclose(fp);
                return false;
            }
            instr_LOCCTR_local += 1;
        }
    }
    mod->instr_size = instr_LOCCTR_local;
    mod->data_size = data_LOCCTR_local;
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
        // printf("Módulo %d: %s\n", module_count, modules[module_count - 1].filename);
        // printf("  Definições (%d):\n", modules[module_count - 1].def_count);
        // for (int d = 0; d < modules[module_count - 1].def_count; d++) {
        //     printf("    %s (rel_addr=%d, %s)\n",
        //            modules[module_count - 1].defs[d].label,
        //            modules[module_count - 1].defs[d].rel_addr,
        //            modules[module_count - 1].defs[d].is_global ? "GLOBAL" : "local");
        // }
        // printf("  Externos (%d):\n", modules[module_count - 1].extern_count);
        // for (int e = 0; e < modules[module_count - 1].extern_count; e++) {
        //     printf("    %s\n", modules[module_count - 1].externs[e]);
        // }
    }
    // Construir EXTAB e atribuir CSADDR
    int instr_cur_base = 0;
    int data_cur_base = 0;
    // Reiniciar EXTAB
    EXTAB_count = 0;
    for (int i = 0; i < module_count; i++) {
        modules[i].instr_CSADDR = instr_cur_base;
        modules[i].data_CSADDR = data_cur_base;
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
                int abs_addr;
                if(modules[i].defs[j].is_data) {
                    abs_addr = data_cur_base + rel;
                } else {
                    abs_addr = instr_cur_base + rel;
                }
                if (!EXTAB_add(sym, abs_addr)) {
                    return false;
                }
            }
        }
        instr_cur_base += modules[i].instr_size;
        data_cur_base += modules[i].data_size;
        // printf("instr_cur_base: %d, data_cur_base: %d\n", instr_cur_base, data_cur_base);
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

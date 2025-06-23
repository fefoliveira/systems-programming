# Makefile

# Nome do binário da VM
BIN = vm_bin

# Nome do binário do montador/linker
ASM_BIN = assembler_bin

# Fontes da VM
VM_SRC = \
    src/vm/main.c \
    src/vm/memory/memory.c \
    src/vm/loader/loader.c \
    src/vm/cpu/cpu.c

# Fontes do montador (single-module e multi-module) + utils
ASM_SRC = \
    src/assembler/assembler.c \
    src/assembler/single-module/first_pass.c \
    src/assembler/single-module/second_pass.c \
    src/assembler/multi-module/linker_first_pass.c \
    src/assembler/multi-module/linker_second_pass.c \
    src/utils/utils.c

.PHONY: all clean run assemble link

all: $(BIN) $(ASM_BIN)

# Compilar VM
$(BIN): $(VM_SRC)
	clang -o $(BIN) $(VM_SRC)

# Compilar montador/linker
$(ASM_BIN): $(ASM_SRC)
	clang -o $(ASM_BIN) $(ASM_SRC)

# Pega argumentos após o alvo (para run/assemble/link)
ARGS := $(wordlist 2, $(words $(MAKECMDGOALS)), $(MAKECMDGOALS))

# Rodar VM: ./vm_bin programs/<nome>.obj
# Uso: make run nome
run: $(BIN)
	@if [ "$(ARGS)" = "" ]; then \
	  echo "Uso: make run <nome>"; exit 1; \
	fi; \
	# @echo "Executando VM: programs/$(ARGS).obj"
	./$(BIN) programs/$(ARGS).obj

# Montar single-module:
# ./assembler_bin programs/<nome>.txt programs/<nome>.obj
# Uso: make assemble nome
assemble: $(ASM_BIN)
	@if [ "$(ARGS)" = "" ]; then \
	  echo "Uso: make assemble <nome>"; exit 1; \
	fi; \
	# @echo "Montando single: programs/$(ARGS).txt → programs/$(ARGS).obj"
	./$(ASM_BIN) programs/$(ARGS).txt programs/$(ARGS).obj

# Linkar múltiplos códigos fonte (módulos):
# Uso: make link mod1 mod2 ...  → gera programs/mod1_mod2.obj
empty :=
space := $(empty) $(empty)
link: $(ASM_BIN)
	@if [ "$(ARGS)" = "" ]; then \
	  echo "Uso: make link <mod1> <mod2> ..."; exit 1; \
	fi; \
	MODS=""; \
	for m in $(ARGS); do \
	  MODS="$$MODS programs/$$m.txt"; \
	done; \
	# OUT_NAME="$(subst $(space),_,$(ARGS))"; \
	OUT_NAME="linked"; \
	echo "Linkando $$MODS → programs/$$OUT_NAME.obj"; \
	./$(ASM_BIN) $$MODS -o programs/$$OUT_NAME.obj

# Limpar executáveis
clean:
	rm -f $(BIN) $(ASM_BIN)

# Evita erro se chamar make <algo>
%:
	@:

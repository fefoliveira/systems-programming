# Makefile

# Nome do binário da VM
BIN = vm_bin

# Nome do binário do montador
ASM_BIN = assembler_bin

# Fontes da VM
VM_SRC = src/vm/main.c src/vm/memory/memory.c src/vm/loader/loader.c src/vm/cpu/cpu.c

# Fontes do montador
ASM_SRC = src/assembler/assembler.c src/assembler/first_pass.c src/assembler/second_pass.c src/utils/utils.c

# Compilar VM
$(BIN): $(VM_SRC)
	clang -o $(BIN) $(VM_SRC)

# Compilar montador
$(ASM_BIN): $(ASM_SRC)
	clang -o $(ASM_BIN) $(ASM_SRC)

# Argumento após run ou assemble
ARGS := $(wordlist 2, $(words $(MAKECMDGOALS)), $(MAKECMDGOALS))

# Rodar VM: ./vm_file programs/<arquivo.obj>
run: $(BIN)
	./$(BIN) programs/$(ARGS).obj

# Montar: ./assembler programs/<arquivo.txt> programs/<arquivo.obj>
assemble: $(ASM_BIN)
	./$(ASM_BIN) programs/$(ARGS).txt programs/$(ARGS).obj

# Limpar executáveis
clean:
	rm -f $(BIN) $(ASM_BIN)

# Impede erros em alvos genéricos
%:
	@:

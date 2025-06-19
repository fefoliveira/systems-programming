# Makefile

# Nome do binário
BIN = vm_file

# Fontes
SRC = src/vm/main.c src/vm/memory/memory.c src/vm/loader/loader.c src/vm/cpu/cpu.c

# Comando para compilar
$(BIN): $(SRC)
	clang -o $(BIN) $(SRC)

# Comando para rodar a VM com argumento
run: $(BIN)
	./$(BIN) $(ARGS)

# Pega o argumento depois de 'run'
ARGS := $(wordlist 2, $(words $(MAKECMDGOALS)), $(MAKECMDGOALS))

# Limpa o executável
clean:
	rm -f $(BIN)

# Impede que o Make tente criar arquivos chamados 'program.txt' etc
%:
	@:

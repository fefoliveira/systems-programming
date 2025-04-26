# Makefile

# Nome do binário
BIN = vm

# Fontes
SRC = src/main.c src/memory/memory.c src/loader/loader.c src/cpu/cpu.c

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

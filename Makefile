# Makefile

# Nome do binário
BIN = vm

# Fontes
SRC = src/main.c src/memory/memory.c src/loader/loader.c src/core/core.c

# Comando para compilar
$(BIN): $(SRC)
	clang -o $(BIN) $(SRC)

# Comando para rodar a VM
run: $(BIN)
	./$(BIN)

# Limpa o executável
clean:
	rm -f $(BIN)

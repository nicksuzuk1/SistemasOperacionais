# Nome do compilador
CC = gcc

# Flags do compilador
CFLAGS = -Wall -Wextra -pthread

# Nome do executável
TARGET = matrix_operations

# Arquivo de origem
SRC = trabalho_so.c

# Regras do Makefile
all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Limpa os arquivos de build
clean:
	rm -f $(TARGET) *.o

# Regra phony para garantir que 'clean' sempre é executado
.PHONY: all clean

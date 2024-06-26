## README

# Matrizes e Multithreading

Este programa realiza operações com matrizes utilizando multithreading para otimizar a execução. Ele é capaz de ler matrizes de arquivos, somar duas matrizes, multiplicar matrizes e reduzir uma matriz somando todos os seus elementos. O programa mede o tempo de execução para cada uma dessas operações.

### Pré-requisitos

- Sistema Operacional: Linux (testado no Ubuntu)
- Compilador GCC

### Compilação

Para compilar o programa, você pode utilizar o `Makefile` fornecido. Execute o seguinte comando no terminal:

```sh
make
```

Isso irá gerar um executável chamado `matrix_operations`.

### Execução

Para executar o programa, utilize o seguinte comando:

```sh
./matrix_operations T n arqA.dat arqB.dat arqC.dat arqD.dat arqE.dat
```

Onde:
- `T`: Número de threads a serem usadas.
- `n`: Tamanho da matriz (n x n).
- `arqA.dat`: Caminho para o arquivo contendo a matriz A.
- `arqB.dat`: Caminho para o arquivo contendo a matriz B.
- `arqC.dat`: Caminho para o arquivo contendo a matriz C.
- `arqD.dat`: Caminho para o arquivo onde a matriz D (resultado da soma de A e B) será salva.
- `arqE.dat`: Caminho para o arquivo onde a matriz E (resultado da multiplicação de D e C) será salva.

### Exemplo de Execução

```sh
./matrix_operations 4 100 matrixA.dat matrixB.dat matrixC.dat matrixD.dat matrixE.dat
```

### Saída

O programa irá imprimir no terminal o tempo gasto em cada operação e o resultado da redução da matriz E. A saída terá o seguinte formato:

```
Reducao: [resultado da redução]
Tempo Soma: [tempo em segundos] segundos
Tempo Multiplicacao: [tempo em segundos] segundos
Tempo Reducao: [tempo em segundos] segundos
Tempo total: [tempo em segundos] segundos
```

### Estrutura do Código

- **readMatrix:** Função para ler uma matriz de um arquivo.
- **writeMatrix:** Função para escrever uma matriz em um arquivo.
- **addMatrices:** Função para somar duas matrizes.
- **multiplyMatrices:** Função para multiplicar duas matrizes.
- **reduceMatrix:** Função para reduzir uma matriz somando todos os seus elementos.
- **main:** Função principal que orquestra a execução das operações de leitura, soma, multiplicação, redução e escrita das matrizes, além de medir o tempo de execução de cada operação.

### Autores
- José Vitor Dutra Antônio
- Nicolas Suzuki
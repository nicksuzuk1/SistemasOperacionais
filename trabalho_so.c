#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

typedef struct {
    int n;
    int **A;
    int **B;
    int **C;
    int **D;
    int **E;
    int start;
    int end;
} ThreadData;

// Essa funcao le uma matriz de um arquivo e a armazena na memoria.
void readMatrix(const char* filename, int **matrix, int n) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            fscanf(file, "%d", &matrix[i][j]);
    fclose(file);
}

// Essa funcao escreve uma matriz em um arquivo.
void writeMatrix(const char* filename, int **matrix, int n) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        perror("Erro ao abrir arquivo");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++)
            fprintf(file, "%d ", matrix[i][j]);
        fprintf(file, "\n");
    }
    fclose(file);
}

// Essa funcao realiza a soma das matrizes A e B, armazenando o resultado na matriz D.
DWORD WINAPI addMatrices(LPVOID arg) {
    ThreadData *data = (ThreadData*) arg;
    for (int i = data->start; i < data->end; i++)
        for (int j = 0; j < data->n; j++)
            data->D[i][j] = data->A[i][j] + data->B[i][j];
    return 0;
}

// Essa funcao realiza a multiplicacao das matrizes D e C, armazenando o resultado na matriz E.
DWORD WINAPI multiplyMatrices(LPVOID arg) {
    ThreadData *data = (ThreadData*) arg;
    for (int i = data->start; i < data->end; i++)
        for (int j = 0; j < data->n; j++) {
            data->E[i][j] = 0;
            for (int k = 0; k < data->n; k++)
                data->E[i][j] += data->D[i][k] * data->C[k][j];
        }
    return 0;
}

// Essa funcao soma todos os elementos da matriz E e retorna o resultado.
DWORD WINAPI reduceMatrix(LPVOID arg) {
    ThreadData *data = (ThreadData*) arg;
    int *result = (int*) malloc(sizeof(int));
    *result = 0;
    for (int i = data->start; i < data->end; i++)
        for (int j = 0; j < data->n; j++)
            *result += data->E[i][j];
    return (DWORD)result;
}

int main(int argc, char* argv[]) {
    // Verifica se o programa recebeu os argumentos corretos
    if (argc != 8) {
        fprintf(stderr, "Uso: %s T n arqA.dat arqB.dat arqC.dat arqD.dat arqE.dat\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Declara as variaveis com os argumentos passados
    int T = atoi(argv[1]);
    int n = atoi(argv[2]);
    const char *arqA = argv[3];
    const char *arqB = argv[4];
    const char *arqC = argv[5];
    const char *arqD = argv[6];
    const char *arqE = argv[7];

    // Alocar memoria para as matrizes
    int **A = (int**) malloc(n * sizeof(int*));
    int **B = (int**) malloc(n * sizeof(int*));
    int **C = (int**) malloc(n * sizeof(int*));
    int **D = (int**) malloc(n * sizeof(int*));
    int **E = (int**) malloc(n * sizeof(int*));
    for (int i = 0; i < n; i++) {
        A[i] = (int*) malloc(n * sizeof(int));
        B[i] = (int*) malloc(n * sizeof(int));
        C[i] = (int*) malloc(n * sizeof(int));
        D[i] = (int*) malloc(n * sizeof(int));
        E[i] = (int*) malloc(n * sizeof(int));
    }

    // Medicao do tempo total de execucao
    clock_t startTotal = clock();

    // Passo 1: Leitura das Matrizes A e B
    readMatrix(arqA, A, n);
    readMatrix(arqB, B, n);

    // Passo 2: Soma das Matrizes A e B = D
    clock_t startSum = clock();
    HANDLE *threads = (HANDLE*) malloc(T * sizeof(HANDLE));
    ThreadData *threadData = (ThreadData*) malloc(T * sizeof(ThreadData));
    int chunkSize = n / T;
    for (int i = 0; i < T; i++) {
        threadData[i].n = n;
        threadData[i].A = A;
        threadData[i].B = B;
        threadData[i].D = D;
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == T - 1) ? n : (i + 1) * chunkSize;
        threads[i] = CreateThread(NULL, 0, addMatrices, &threadData[i], 0, NULL);
    }
    WaitForMultipleObjects(T, threads, TRUE, INFINITE);
    clock_t endSum = clock();

    // Passo 3: Gravacao da Matriz D
    writeMatrix(arqD, D, n);

    // Passo 4: Leitura da Matriz C
    readMatrix(arqC, C, n);

    // Passo 5: Multiplicacao das Matrizes D e C = E
    clock_t startMult = clock();
    for (int i = 0; i < T; i++) {
        threadData[i].C = C;
        threadData[i].E = E;
        threads[i] = CreateThread(NULL, 0, multiplyMatrices, &threadData[i], 0, NULL);
    }
    WaitForMultipleObjects(T, threads, TRUE, INFINITE);
    clock_t endMult = clock();

    // Passo 6: Gravacao da Matriz E
    writeMatrix(arqE, E, n);

    // Passo 7: Reducao da Matriz E
    clock_t startRed = clock();
    int finalSum = 0;
    for (int i = 0; i < T; i++) {
        int *partialSum;
        threads[i] = CreateThread(NULL, 0, reduceMatrix, &threadData[i], 0, NULL);
        WaitForSingleObject(threads[i], INFINITE);
        GetExitCodeThread(threads[i], (LPDWORD)&partialSum);
        finalSum += *partialSum;
        free(partialSum);
    }
    clock_t endRed = clock();

    // Medicao do tempo final
    clock_t endTotal = clock();

    // Exibicao dos resultados
    printf("Reducao: %d\n", finalSum);
    printf("Tempo Soma: %f segundos\n", (double)(endSum - startSum) / CLOCKS_PER_SEC);
    printf("Tempo Multiplicacao: %f segundos\n", (double)(endMult - startMult) / CLOCKS_PER_SEC);
    printf("Tempo Reducao: %f segundos\n", (double)(endRed - startRed) / CLOCKS_PER_SEC);
    printf("Tempo total: %f segundos\n", (double)(endTotal - startTotal) / CLOCKS_PER_SEC);

    // Liberacao da memoria alocada
    for (int i = 0; i < n; i++) {
        free(A[i]);
        free(B[i]);
        free(C[i]);
        free(D[i]);
        free(E[i]);
    }
    free(A);
    free(B);
    free(C);
    free(D);
    free(E);
    free(threads);
    free(threadData);

    return 0;
}

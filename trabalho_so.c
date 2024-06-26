#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
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

// Função para ler uma matriz de um arquivo e armazená-la na memória.
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

// Função para escrever uma matriz em um arquivo.
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

// Função para realizar a soma das matrizes A e B, armazenando o resultado na matriz D.
void* addMatrices(void* arg) {
    ThreadData *data = (ThreadData*) arg;
    for (int i = data->start; i < data->end; i++)
        for (int j = 0; j < data->n; j++)
            data->D[i][j] = data->A[i][j] + data->B[i][j];
    return NULL;
}

// Função para realizar a multiplicação das matrizes D e C, armazenando o resultado na matriz E.
void* multiplyMatrices(void* arg) {
    ThreadData *data = (ThreadData*) arg;
    for (int i = data->start; i < data->end; i++)
        for (int j = 0; j < data->n; j++) {
            data->E[i][j] = 0;
            for (int k = 0; k < data->n; k++)
                data->E[i][j] += data->D[i][k] * data->C[k][j];
        }
    return NULL;
}

// Função para somar todos os elementos da matriz E e retornar o resultado.
void* reduceMatrix(void* arg) {
    ThreadData *data = (ThreadData*) arg;
    int *result = (int*) malloc(sizeof(int));
    if (!result) {
        perror("Erro ao alocar memória para resultado");
        exit(EXIT_FAILURE);
    }
    *result = 0;
    for (int i = data->start; i < data->end; i++)
        for (int j = 0; j < data->n; j++)
            *result += data->E[i][j];
    pthread_exit(result);
}

int main(int argc, char* argv[]) {
    // Verifica se o programa recebeu os argumentos corretos
    if (argc != 8) {
        fprintf(stderr, "Uso: %s T n arqA.dat arqB.dat arqC.dat arqD.dat arqE.dat\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Declara as variáveis com os argumentos passados
    int T = atoi(argv[1]);
    int n = atoi(argv[2]);
    const char *arqA = argv[3];
    const char *arqB = argv[4];
    const char *arqC = argv[5];
    const char *arqD = argv[6];
    const char *arqE = argv[7];

    // Aloca memória para as matrizes
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

    // Medição do tempo total de execução
    clock_t startTotal = clock();

    // Passo 1: Leitura das Matrizes A e B
    readMatrix(arqA, A, n);
    readMatrix(arqB, B, n);

    // Passo 2: Soma das Matrizes A e B = D
    clock_t startSum = clock();
    pthread_t *threads = (pthread_t*) malloc(T * sizeof(pthread_t));
    ThreadData *threadData = (ThreadData*) malloc(T * sizeof(ThreadData));
    int chunkSize = n / T;

    for (int i = 0; i < T; i++) {
        threadData[i].n = n;
        threadData[i].A = A;
        threadData[i].B = B;
        threadData[i].D = D;
        threadData[i].start = i * chunkSize;
        threadData[i].end = (i == T - 1) ? n : (i + 1) * chunkSize;
        pthread_create(&threads[i], NULL, addMatrices, &threadData[i]);
    }
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_t endSum = clock();

    // Passo 3: Gravação da Matriz D
    writeMatrix(arqD, D, n);

    // Passo 4: Leitura da Matriz C
    readMatrix(arqC, C, n);

    // Passo 5: Multiplicação das Matrizes D e C = E
    clock_t startMult = clock();
    for (int i = 0; i < T; i++) {
        threadData[i].C = C;
        threadData[i].E = E;
        pthread_create(&threads[i], NULL, multiplyMatrices, &threadData[i]);
    }
    for (int i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_t endMult = clock();

    // Passo 6: Gravação da Matriz E
    writeMatrix(arqE, E, n);

    // Passo 7: Redução da Matriz E
    clock_t startRed = clock();
    int finalSum = 0;
    for (int i = 0; i < T; i++) {
        int *partialSum;
        pthread_create(&threads[i], NULL, reduceMatrix, &threadData[i]);
        pthread_join(threads[i], (void**)&partialSum);
        finalSum += *partialSum;
        free(partialSum);
    }
    clock_t endRed = clock();

    // Medição do tempo final
    clock_t endTotal = clock();

    // Exibição dos resultados
    printf("Reducao: %d\n", finalSum);
    printf("Tempo Soma: %f segundos\n", (double)(endSum - startSum) / CLOCKS_PER_SEC);
    printf("Tempo Multiplicacao: %f segundos\n", (double)(endMult - startMult) / CLOCKS_PER_SEC);
    printf("Tempo Reducao: %f segundos\n", (double)(endRed - startRed) / CLOCKS_PER_SEC);
    printf("Tempo total: %f segundos\n", (double)(endTotal - startTotal) / CLOCKS_PER_SEC);

    // Liberação da memória alocada
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


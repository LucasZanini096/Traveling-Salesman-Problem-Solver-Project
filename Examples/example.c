#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

// Definição da estrutura para o solucionador TSP
typedef struct {
    double** adjacencyMatrix;
    int matrixSize;
    unsigned int seed;
} TSPSolver;

// Protótipos de funções
TSPSolver* createTSPSolver(unsigned int seed);
void destroyTSPSolver(TSPSolver* solver);
void loadAdjacencyMatrix(TSPSolver* solver, FILE* inputFile);
int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts);
double calculateTourLength(TSPSolver* solver, const int* tour);
int* generateRandomTour(TSPSolver* solver);
void twoOptSwap(const int* tour, int* newTour, int i, int j, int tourSize);
int* shotgunHillClimbing(TSPSolver* solver, int numIterations, int numRestarts);
void hillClimb(TSPSolver* solver, int numIterations, int** bestTourPtr, double* bestLengthPtr);
double** parseCSVMatrix(FILE* inputFile, int* size);
int isSquareMatrix(double** matrix, int size);
void swap(int* a, int* b);
void shuffleArray(int* array, int size, unsigned int* seed);
void reverseSubArray(int* array, int start, int end);

// Função principal
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    // Abre o arquivo de entrada
    FILE* inputFile = fopen(argv[1], "r");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not open input file %s\n", argv[1]);
        return 1;
    }

    char line[1024];
    int numIterations, numRestarts;
    unsigned int seed;
    
    // Lê os parâmetros da primeira linha do arquivo
    if (fgets(line, sizeof(line), inputFile) == NULL) {
        fprintf(stderr, "Error: Could not read input parameters\n");
        fclose(inputFile);
        return 1;
    }
    
    if (sscanf(line, "%d %d %u", &numIterations, &numRestarts, &seed) != 3) {
        fprintf(stderr, "Error: Invalid input parameters\n");
        fclose(inputFile);
        return 1;
    }
    
    // Cria o solucionador
    TSPSolver* solver = createTSPSolver(seed);
    if (!solver) {
        fprintf(stderr, "Error: Failed to create TSP solver\n");
        fclose(inputFile);
        return 1;
    }
    
    // Carrega a matriz de adjacência do arquivo
    loadAdjacencyMatrix(solver, inputFile);
    
    // Fecha o arquivo de entrada
    fclose(inputFile);
    
    // Resolve o problema do caixeiro viajante
    int* bestTour = solveTSP(solver, numIterations, numRestarts);
    if (!bestTour) {
        fprintf(stderr, "Error: Failed to solve TSP\n");
        destroyTSPSolver(solver);
        return 1;
    }
    
    // Exibe o resultado
    printf("Best tour found: ");
    for (int i = 0; i < solver->matrixSize; i++) {
        printf("%d ", bestTour[i]);
    }
    printf("\nTour length: %f\n", calculateTourLength(solver, bestTour));
    
    // Libera a memória
    free(bestTour);
    destroyTSPSolver(solver);
    
    return 0;
}

// Cria um novo solucionador TSP
TSPSolver* createTSPSolver(unsigned int seed) {
    TSPSolver* solver = (TSPSolver*)malloc(sizeof(TSPSolver));
    if (!solver) return NULL;
    
    solver->adjacencyMatrix = NULL;
    solver->matrixSize = 0;
    solver->seed = seed;
    
    return solver;
}

// Carrega a matriz de adjacência do arquivo de entrada
void loadAdjacencyMatrix(TSPSolver* solver, FILE* inputFile) {
    solver->adjacencyMatrix = parseCSVMatrix(inputFile, &solver->matrixSize);
    
    if (!solver->adjacencyMatrix || !isSquareMatrix(solver->adjacencyMatrix, solver->matrixSize)) {
        fprintf(stderr, "Invalid adjacency matrix in CSV file\n");
        exit(1);
    }
}

// Analisa a matriz CSV do arquivo
double** parseCSVMatrix(FILE* inputFile, int* size) {
    double** matrix = NULL;
    char line[1024];
    int rows = 0, cols = 0;
    long filePos = ftell(inputFile);
    
    // Conta o número de linhas
    while (fgets(line, sizeof(line), inputFile) != NULL) {
        rows++;
    }
    
    // Reinicia a posição do arquivo para a posição após os parâmetros
    fseek(inputFile, filePos, SEEK_SET);
    
    // Aloca memória para a matriz
    matrix = (double**)malloc(rows * sizeof(double*));
    if (!matrix) return NULL;
    
    // Lê cada linha e converte para valores reais
    for (int i = 0; i < rows; i++) {
        if (fgets(line, sizeof(line), inputFile) == NULL) {
            // Erro de leitura, libera memória alocada
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
        
        // Remove o caractere de nova linha se existir
        int len = strlen(line);
        if (len > 0 && (line[len-1] == '\n' || line[len-1] == '\r')) {
            line[len-1] = '\0';
        }
        
        // Conta o número de colunas na primeira linha
        if (i == 0) {
            char* temp = line;
            cols = 1; // Pelo menos uma coluna
            while (*temp) {
                if (*temp == ',') cols++;
                temp++;
            }
        }
        
        // Aloca memória para as colunas
        matrix[i] = (double*)malloc(cols * sizeof(double));
        if (!matrix[i]) {
            // Erro de alocação, libera memória alocada
            for (int j = 0; j < i; j++) {
                free(matrix[j]);
            }
            free(matrix);
            return NULL;
        }
        
        // Analisa os valores separados por vírgula
        char* token = strtok(line, ",");
        for (int j = 0; j < cols && token != NULL; j++) {
            matrix[i][j] = atof(token);
            token = strtok(NULL, ",");
        }
    }
    
    *size = rows;
    return matrix;
}

// Verifica se a matriz é quadrada
int isSquareMatrix(double** matrix, int size) {
    return matrix != NULL && size > 0;
}

// Libera a memória do solucionador
void destroyTSPSolver(TSPSolver* solver) {
    if (solver) {
        if (solver->adjacencyMatrix) {
            for (int i = 0; i < solver->matrixSize; i++) {
                free(solver->adjacencyMatrix[i]);
            }
            free(solver->adjacencyMatrix);
        }
        free(solver);
    }
}

// Resolve o problema do caixeiro viajante
int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts) {
    return shotgunHillClimbing(solver, numIterations, numRestarts);
}

// Calcula o comprimento de um tour
// Utilização de um reducer ( OPENMP ) para paralelizar o cálculo
// do comprimento do tour
double calculateTourLength(TSPSolver* solver, const int* tour) {
    double length = 0.0;
    for (int i = 0; i < solver->matrixSize; i++) {
        int from = tour[i];
        int to = tour[(i + 1) % solver->matrixSize];
        length += solver->adjacencyMatrix[from][to];
    }
    return length;
}

// Gera um tour aleatório
int* generateRandomTour(TSPSolver* solver) {
    int* tour = (int*)malloc(solver->matrixSize * sizeof(int));
    if (!tour) return NULL;
    
    // Inicializa o tour com índices sequenciais
    for (int i = 0; i < solver->matrixSize; i++) {
        tour[i] = i;
    }
    
    // Embaralha todos os índices, exceto o primeiro
    shuffleArray(tour + 1, solver->matrixSize - 1, &solver->seed);
    
    return tour;
}

// Embaralha um array
void shuffleArray(int* array, int size, unsigned int* seed) {
    for (int i = size - 1; i > 0; i--) {
        // Usa o algoritmo Fisher-Yates
        int j = rand_r(seed) % (i + 1);
        swap(&array[i], &array[j]);
    }
}

// Troca dois elementos
void swap(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

// Realiza a operação 2-opt swap em um tour
void twoOptSwap(const int* tour, int* newTour, int i, int j, int tourSize) {
    // Copia o tour original
    for (int k = 0; k < tourSize; k++) {
        newTour[k] = tour[k];
    }
    
    // Reverte a sub-sequência do índice i até j
    reverseSubArray(newTour, i, j);
}

// Reverte uma sub-sequência de um array
void reverseSubArray(int* array, int start, int end) {
    while (start < end) {
        swap(&array[start], &array[end]);
        start++;
        end--;
    }
}

// Implementa o shotgun hill climbing
// Paralização com o OPENMP os restarts
int* shotgunHillClimbing(TSPSolver* solver, int numIterations, int numRestarts) {
    int* bestTour = NULL;
    double bestLength = DBL_MAX;
    
    for (int restart = 0; restart < numRestarts; restart++) {
        int* currentTour = NULL;
        double currentLength = 0.0;
        
        // Executa hill climbing
        hillClimb(solver, numIterations, &currentTour, &currentLength);
        
        if (currentLength < bestLength) {
            // Atualiza o melhor tour encontrado
            if (bestTour) free(bestTour);
            bestTour = currentTour;
            bestLength = currentLength;
        } else {
            // Libera o tour atual se não for o melhor
            free(currentTour);
        }
    }
    
    return bestTour;
}

// Implementa o algoritmo hill climbing
// Paralização com o OPENMP
void hillClimb(TSPSolver* solver, int numIterations, int** bestTourPtr, double* bestLengthPtr) {
    int* currentTour = generateRandomTour(solver);
    double currentLength = calculateTourLength(solver, currentTour);
    int* newTour = (int*)malloc(solver->matrixSize * sizeof(int));
    
    for (int iter = 0; iter < numIterations; iter++) {
        int improvement = 0;
        
        for (int i = 1; i < solver->matrixSize - 1 && !improvement; i++) {
            for (int j = i + 1; j < solver->matrixSize && !improvement; j++) {
                twoOptSwap(currentTour, newTour, i, j, solver->matrixSize);
                double newLength = calculateTourLength(solver, newTour);
                
                if (newLength < currentLength) {
                    // Troca o tour atual pelo novo
                    int* tempTour = currentTour;
                    currentTour = newTour;
                    newTour = tempTour;
                    currentLength = newLength;
                    improvement = 1;
                }
            }
        }
        
        if (!improvement) break;
    }
    
    free(newTour);
    
    *bestTourPtr = currentTour;
    *bestLengthPtr = currentLength;
}
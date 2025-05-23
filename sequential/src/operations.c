#include "../include/operations.h"


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

// Reverte uma sub-sequência de um array
void reverseSubArray(int* array, int start, int end) {
    while (start < end) {
        swap(&array[start], &array[end]);
        start++;
        end--;
    }
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

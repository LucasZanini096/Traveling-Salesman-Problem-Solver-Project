#include "../include/tspSolver.h"
#include "../include/operations.h"


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

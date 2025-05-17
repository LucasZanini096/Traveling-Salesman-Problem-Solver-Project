#include "../include/tour.h"
#include "../include/operations.h"
#include "../include/tspSolver.h"


int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts) {
    return shotgunHillClimbing(solver, numIterations, numRestarts);
}

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
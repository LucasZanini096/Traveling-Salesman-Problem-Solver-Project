#include "../include/tour.h"
#include "../include/operations.h"
#include "../include/tspSolver.h"


int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts) {
    return shotgunHillClimbing(solver, numIterations, numRestarts);
}

//Aplicação do reducer do OPENMP
// Grao de paralelismo: cada iteração do loop
// O loop é paralelizado, mas o resultado final é reduzido para o melhor tour encontrado
double calculateTourLength(TSPSolver* solver, const int* tour) {
    double length = 0.0;

    // Paraleliza o loop com OpenMP
    // Cada thread calcula a soma parcial do comprimento do tour
    // omp_set_num_threads(NUM_THREADS);
    #pragma omp parallel for reduction(+:length)
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
    
    // omp_set_num_threads(NUM_THREADS);
    #pragma omp parallel for   
        for (int i = 0; i < solver->matrixSize; i++) {
            tour[i] = i;
        }
    // // Inicializa o tour com índices sequenciais
    // for (int i = 0; i < solver->matrixSize; i++) {
    //     tour[i] = i;
    // }
    
    // Embaralha todos os índices, exceto o primeiro
    shuffleArray(tour + 1, solver->matrixSize - 1, &solver->seed);
    
    return tour;
}

//Paralelizavel -> cada task pode ser executada em paralelo
// Grão de paralelismo: cada reinicialização do algoritmo
int* shotgunHillClimbing(TSPSolver* solver, int numIterations, int numRestarts) {
    int* bestTour = NULL;
    double bestLength = DBL_MAX;
    
    // omp_set_num_threads(NUM_THREADS);
     // Define o número de threads
        // Inicializa o melhor tour com NULL e comprimento máximo
        if (bestTour == NULL) {
            bestTour = (int*)malloc(solver->matrixSize * sizeof(int));
            if (!bestTour) {
                fprintf(stderr, "Memory allocation failed\n");
                exit(1);
            }
        }

        bestLength = DBL_MAX;
  

    #pragma omp parallel for shared(bestTour, bestLength)
    for (int restart = 0; restart < numRestarts; restart++) {
        int* currentTour = NULL;
        double currentLength = 0.0;
        
        // Executa hill climbing
        hillClimb(solver, numIterations, &currentTour, &currentLength);
        
        // Zona crítica para atualizar o melhor tour encontrado
        #pragma omp critical
        {
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
    }
    
    return bestTour;
}

// Implementa o algoritmo hill climbing
void hillClimb(TSPSolver* solver, int numIterations, int** bestTourPtr, double* bestLengthPtr) {
    int* currentTour = generateRandomTour(solver);
    double currentLength = calculateTourLength(solver, currentTour);
    
    for (int iter = 0; iter < numIterations; iter++) {
        // Variáveis compartilhadas para encontrar a melhor melhoria
        double bestNewLength = currentLength;
        int* bestNewTour = NULL;
        int foundImprovement = 0;
        
        #pragma omp parallel //Definição de região paralela
        {
            #pragma omp single //Apenas 1 thread executa o bloco
            {
                // UMA thread cria todas as tasks
                for (int i = 1; i < solver->matrixSize - 1; i++) {
                    for (int j = i + 1; j < solver->matrixSize; j++) {
                        
                        // Cria uma TASK para cada combinação (i,j)
                        #pragma omp task firstprivate(i, j) shared(bestNewLength, bestNewTour, foundImprovement)
                        {
                            // Cada task trabalha com sua própria cópia - rota
                            int* localTour = (int*)malloc(solver->matrixSize * sizeof(int));
                            
                            if (!localTour) {
                                fprintf(stderr, "Memory allocation failed\n");
                                exit(1);
                            }

                            // Aplica a operação 2-opt para esta combinação específica
                            twoOptSwap(currentTour, localTour, i, j, solver->matrixSize);
                            double localLength = calculateTourLength(solver, localTour);
                            
                            // Seção crítica para comparar com a melhor solução
                            #pragma omp critical
                            {
                                if (localLength < bestNewLength) {
                                    // Libera o tour anterior se existir
                                    if (bestNewTour) free(bestNewTour);
                                    
                                    // Atualiza a melhor solução
                                    bestNewTour = localTour;
                                    bestNewLength = localLength;
                                    foundImprovement = 1;
                                } else {
                                    // Libera o tour local se não melhorou
                                    free(localTour);
                                }
                            }
                        }
                    }
                }
            }
            // Espera todas as tasks terminarem
            #pragma omp taskwait
        }
        
        // Verifica se houve melhoria
        if (foundImprovement) {
            free(currentTour);
            currentTour = bestNewTour;
            currentLength = bestNewLength;
        } else {
            // Se não houve melhoria, para o algoritmo
            if (bestNewTour) free(bestNewTour);
            break;
        }
    }
    
    *bestTourPtr = currentTour;
    *bestLengthPtr = currentLength;
}
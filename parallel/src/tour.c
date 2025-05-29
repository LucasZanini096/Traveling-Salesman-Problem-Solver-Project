#include "../include/tour.h"
#include "../include/operations.h"
#include "../include/tspSolver.h"


int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts) {
    return shotgunHillClimbing(solver, numIterations, numRestarts);
}

//Aplicação do reducer do OPENMP
// Grao de paralelismo: cada iteração do loop
// O loop é paralelizado, mas o resultado final é reduzido para o melhor tour encontrado

// Versão otimizada com diferentes estratégias baseadas no tamanho
double calculateTourLength(TSPSolver* solver, const int* tour) {
    const int size = solver->matrixSize;
    double length = 0.0;
    
    if (size < 50) {
        // Para problemas pequenos, execução sequencial pode ser mais eficiente
        for (int i = 0; i < size; i++) {
            int from = tour[i];
            int to = tour[(i + 1) % size];
            length += solver->adjacencyMatrix[from][to];
        }
    } else if (size < 200) {
        // Para problemas médios, usar paralelização simples
        #pragma omp parallel for reduction(+:length) schedule(static)
        for (int i = 0; i < size; i++) {
            int from = tour[i];
            int to = tour[(i + 1) % size];
            length += solver->adjacencyMatrix[from][to];
        }
    } else {
        // Para problemas grandes, usar chunking manual para reduzir overhead
        const int num_threads = omp_get_num_threads();
        const int chunk_size = (size / num_threads) + 1;
        
        #pragma omp parallel reduction(+:length)
        {
            #pragma omp for schedule(static, chunk_size) nowait
            for (int i = 0; i < size; i++) {
                int from = tour[i];
                int to = tour[(i + 1) % size];
                length += solver->adjacencyMatrix[from][to];
            }
        }
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
    
    // Usar taskloop para melhor balanceamento de carga
    #pragma omp parallel
    {
        #pragma omp single
        {
            // Usar taskloop com grainsize adaptativo baseado no número de threads
            int grainsize = (numRestarts / omp_get_num_threads()) + 1;
            if (grainsize < 1) grainsize = 1;
            if (grainsize > 10) grainsize = 10; // Evitar grãos muito grandes
            
            #pragma omp taskloop grainsize(grainsize) \
                                firstprivate(numIterations) \
                                shared(bestTour, bestLength) \
                                final(numRestarts < 20)
            for (int restart = 0; restart < numRestarts; restart++) {
                int* currentTour = NULL;
                double currentLength = 0.0;
                
                // Usar seed diferente para cada restart para melhor diversidade
                TSPSolver localSolver = *solver;
                localSolver.seed = solver->seed + restart * 1000;
                
                // Executa hill climbing
                hillClimb(&localSolver, numIterations, &currentTour, &currentLength);
                
                // Zona crítica otimizada - minimize o tempo dentro dela
                #pragma omp critical(update_best)
                {
                    if (currentLength < bestLength) {
                        if (bestTour) free(bestTour);
                        bestTour = currentTour;
                        bestLength = currentLength;
                    } else {
                        free(currentTour);
                    }
                }
            }
        }
    }
    
    return bestTour;
}

// Implementa o algoritmo hill climbing
void hillClimb(TSPSolver* solver, int numIterations, int** bestTourPtr, double* bestLengthPtr) {
    int* currentTour = generateRandomTour(solver);
    double currentLength = calculateTourLength(solver, currentTour);
    
    // Definir tamanho mínimo de bloco para evitar overhead excessivo
    const int MIN_BLOCK_SIZE = 100;
    const int n = solver->matrixSize;
    const int total_combinations = (n-2)*(n-1)/2;
    
    for (int iter = 0; iter < numIterations; iter++) {
        double bestNewLength = currentLength;
        int* bestNewTour = NULL;
        int foundImprovement = 0;
        
        #pragma omp parallel
        {
            #pragma omp single
            {
                // Agrupar combinações em blocos maiores para reduzir overhead
                if (total_combinations > MIN_BLOCK_SIZE) {
                    // Processar em blocos de tamanho adequado
                    int block_size = (total_combinations / omp_get_num_threads()) + 1;
                    if (block_size < MIN_BLOCK_SIZE) block_size = MIN_BLOCK_SIZE;
                    
                    for (int block_start = 0; block_start < total_combinations; block_start += block_size) {
                        int block_end = (block_start + block_size < total_combinations) ? 
                                       block_start + block_size : total_combinations;
                        
                        #pragma omp task firstprivate(block_start, block_end) \
                                        final(block_end - block_start < 50) \
                                        mergeable \
                                        shared(bestNewLength, bestNewTour, foundImprovement)
                        {
                            // Variáveis locais para esta task
                            double local_best_length = currentLength;
                            int* local_best_tour = NULL;
                            int local_found = 0;
                            
                            // Processar bloco de combinações
                            int combo = 0;
                            for (int i = 1; i < n - 1 && combo < block_end; i++) {
                                for (int j = i + 1; j < n && combo < block_end; j++) {
                                    if (combo >= block_start) {
                                        // Processa esta combinação
                                        int* localTour = (int*)malloc(n * sizeof(int));
                                        if (!localTour) continue;
                                        
                                        twoOptSwap(currentTour, localTour, i, j, n);
                                        double localLength = calculateTourLength(solver, localTour);
                                        
                                        if (localLength < local_best_length) {
                                            if (local_best_tour) free(local_best_tour);
                                            local_best_tour = localTour;
                                            local_best_length = localLength;
                                            local_found = 1;
                                        } else {
                                            free(localTour);
                                        }
                                    }
                                    combo++;
                                }
                            }
                            
                            // Atualiza resultado global apenas uma vez por task
                            if (local_found) {
                                #pragma omp critical
                                {
                                    if (local_best_length < bestNewLength) {
                                        if (bestNewTour) free(bestNewTour);
                                        bestNewTour = local_best_tour;
                                        bestNewLength = local_best_length;
                                        foundImprovement = 1;
                                    } else {
                                        free(local_best_tour);
                                    }
                                }
                            }
                        }
                    }
                } else {
                    // Para problemas pequenos, usar abordagem original mas otimizada
                    for (int i = 1; i < n - 1; i++) {
                        #pragma omp task firstprivate(i) final(n < 20) mergeable \
                                        shared(bestNewLength, bestNewTour, foundImprovement)
                        {
                            double local_best = currentLength;
                            int* local_tour = NULL;
                            
                            for (int j = i + 1; j < n; j++) {
                                int* testTour = (int*)malloc(n * sizeof(int));
                                if (!testTour) continue;
                                
                                twoOptSwap(currentTour, testTour, i, j, n);
                                double testLength = calculateTourLength(solver, testTour);
                                
                                if (testLength < local_best) {
                                    if (local_tour) free(local_tour);
                                    local_tour = testTour;
                                    local_best = testLength;
                                } else {
                                    free(testTour);
                                }
                            }
                            
                            if (local_tour) {
                                #pragma omp critical
                                {
                                    if (local_best < bestNewLength) {
                                        if (bestNewTour) free(bestNewTour);
                                        bestNewTour = local_tour;
                                        bestNewLength = local_best;
                                        foundImprovement = 1;
                                    } else {
                                        free(local_tour);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            #pragma omp taskwait
        }
        
        if (foundImprovement) {
            free(currentTour);
            currentTour = bestNewTour;
            currentLength = bestNewLength;
        } else {
            if (bestNewTour) free(bestNewTour);
            break;
        }
    }
    
    *bestTourPtr = currentTour;
    *bestLengthPtr = currentLength;
}
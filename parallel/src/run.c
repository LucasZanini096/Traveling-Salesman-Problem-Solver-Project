#include "../include/run.h"

// Função que executa a versão sequencial
void* run_sequential_tsp(void* arg) {
    TSPTestParams* params = (TSPTestParams*)arg;
    
    // Desabilita explicitamente o OpenMP (por precaução)
    omp_set_num_threads(1);
    
    // Executa o algoritmo sequencial
    int* bestTour = solveTSP(params->solver, params->numIterations, params->numRestarts);
    
    // Libera a memória (importante não liberar o solver, pois ele será usado na versão paralela)
    if (bestTour) {
        free(bestTour);
    }
    
    return NULL;
}


// Função que executa a versão paralela
void* run_parallel_tsp(void* arg) {
    TSPTestParams* params = (TSPTestParams*)arg;
    
    // O número de threads já deve estar configurado pela função run_performance_test
    
    // Executa o algoritmo paralelo (mesmo código, mas com OpenMP ativado)
    int* bestTour = solveTSP(params->solver, params->numIterations, params->numRestarts);
    
    // Libera a memória
    if (bestTour) {
        free(bestTour);
    }
    
    return NULL;
}
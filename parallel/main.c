#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "src/performance_metrics.c"
#include "src/tspSolver.c"
#include "src/operations.c"
#include "src/tour.c"
#include "src/run.c"

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
    
    // Prepara os parâmetros para os testes
    TSPTestParams params;
    params.solver = solver;
    params.numIterations = numIterations;
    params.numRestarts = numRestarts;

    // Execute testes para diferentes números de threads
    printf("Matriz de distância: %dx%d\n", solver->matrixSize, solver->matrixSize);
    printf("Parâmetros: numIterations=%d, numRestarts=%d, seed=%u\n\n", 
           numIterations, numRestarts, seed);
    
    printf("Executando testes de desempenho...\n");
    
    // Array com os números de threads a testar
    int thread_counts[] = {1, 2, 4, 8}; // Ajuste conforme necessário
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    
    // Executar o teste para cada configuração de threads
    for (int i = 0; i < num_tests; i++) {
        int num_threads = thread_counts[i];
        
        printf("\nTeste com %d threads:\n", num_threads);
        
        // Executa o teste de desempenho
        PerformanceMetrics metrics = run_performance_test(
            run_sequential_tsp, run_parallel_tsp, num_threads, &params);
        
        // Imprime os resultados
        print_metrics(metrics);
    }
    
    // Libera a memória
    destroyTSPSolver(solver);
    
    return 0;
}
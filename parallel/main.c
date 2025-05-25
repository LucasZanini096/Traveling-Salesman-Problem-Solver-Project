#include "src/tspSolver.c"
#include "src/operations.c"
#include "src/tour.c"

// Função principal
int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_file>\n", argv[0]);
        return 1;
    }

    double startTime, endTime;

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
    printf("Solving TSP with OpenMP parallelization...\n");
    printf("Problem size: %d cities\n", solver->matrixSize);
    printf("Iterations: %d, Restarts: %d\n", numIterations, numRestarts);
    printf("Threads available: %d\n", omp_get_max_threads());
    
    // Inicia o cronômetro
    startTime = omp_get_wtime();
    int* bestTour = solveTSP(solver, numIterations, numRestarts);
    // Para o cronômetro
    endTime = omp_get_wtime();
    
    if (!bestTour) {
        fprintf(stderr, "Error: Failed to solve TSP\n");
        destroyTSPSolver(solver);
        return 1;
    }
    
    // Exibe o resultado
    printf("\n=== RESULTS ===\n");
    printf("Best tour found: ");
    for (int i = 0; i < solver->matrixSize; i++) {
        printf("%d ", bestTour[i]);
    }
    printf("\nTour length: %.6f\n", calculateTourLength(solver, bestTour));
    printf("Execution time: %.6f seconds\n", endTime - startTime);
    
    // Calcula e exibe estatísticas adicionais
    double tour_length = calculateTourLength(solver, bestTour);
    printf("\n=== PERFORMANCE METRICS ===\n");
    printf("Cities processed: %d\n", solver->matrixSize);
    printf("Total iterations: %d\n", numIterations * numRestarts);
    printf("Time per iteration: %.6f ms\n", 
           (endTime - startTime) * 1000.0 / (numIterations * numRestarts));
    printf("Threads used: %d\n", omp_get_max_threads());
    
    // Libera a memória
    free(bestTour);
    destroyTSPSolver(solver);
    
    return 0;
}
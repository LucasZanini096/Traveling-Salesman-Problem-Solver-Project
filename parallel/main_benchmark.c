#include "src/tspSolver.c"
#include "src/operations.c"
#include "src/tour.c"
#include "src/benchmark.c"

// Função principal com opção de benchmark
int main(int argc, char* argv[]) {

    double startTime, endTime;

    if (argc < 2 || argc > 3) {
        fprintf(stderr, "Usage: %s <input_file> [--benchmark]\n", argv[0]);
        fprintf(stderr, "  Normal mode: %s input.txt\n", argv[0]);
        fprintf(stderr, "  Benchmark mode: %s input.txt --benchmark\n", argv[0]);
        return 1;
    }

    // Verifica se é modo benchmark
    int benchmark_mode = 0;
    if (argc == 3 && strcmp(argv[2], "--benchmark") == 0) {
        benchmark_mode = 1;
    }

    if (benchmark_mode) {
        // Modo benchmark completo
        char csv_filename[256];
        snprintf(csv_filename, sizeof(csv_filename), "benchmark_results_%ld.csv", time(NULL));
        
        printf("Running TSP Benchmark Suite\n");
        printf("Input file: %s\n", argv[1]);
        printf("Output CSV: %s\n\n", csv_filename);
        
        runFullBenchmark(argv[1], csv_filename);
        return 0;
    }

    // Modo normal de execução
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
    
    // Salva resultado simples em CSV para modo normal
    char simple_csv[256];
    snprintf(simple_csv, sizeof(simple_csv), "result_%ld.csv", time(NULL));
    FILE* csv_file = fopen(simple_csv, "w");
    if (csv_file) {
        fprintf(csv_file, "Threads,Time_s,Tour_Length,Cities,Iterations,Restarts\n");
        fprintf(csv_file, "%d,%.6f,%.6f,%d,%d,%d\n",
                omp_get_max_threads(), endTime - startTime, tour_length,
                solver->matrixSize, numIterations, numRestarts);
        fclose(csv_file);
        printf("Simple result saved to: %s\n", simple_csv);
    }
    
    // Libera a memória
    free(bestTour);
    destroyTSPSolver(solver);
    
    return 0;
}
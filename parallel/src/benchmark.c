#include "../include/benchmark.h"
#include "../include/tspSolver.h"
#include "../include/tour.h"

// Cria uma nova suite de benchmark
BenchmarkSuite* createBenchmarkSuite(const char* problem_name) {
    BenchmarkSuite* suite = (BenchmarkSuite*)malloc(sizeof(BenchmarkSuite));
    if (!suite) return NULL;
    
    suite->results = (BenchmarkResult*)malloc(10 * sizeof(BenchmarkResult));
    if (!suite->results) {
        free(suite);
        return NULL;
    }
    
    suite->count = 0;
    suite->capacity = 10;
    suite->sequential_time = 0.0;
    suite->problem_name = (char*)malloc(strlen(problem_name) + 1);
    strcpy(suite->problem_name, problem_name);
    
    return suite;
}

// Adiciona um resultado ao benchmark
void addBenchmarkResult(BenchmarkSuite* suite, BenchmarkResult result) {
    if (!suite) return;
    
    // Redimensiona o array se necessário
    if (suite->count >= suite->capacity) {
        suite->capacity *= 2;
        suite->results = (BenchmarkResult*)realloc(suite->results, 
                                                  suite->capacity * sizeof(BenchmarkResult));
    }
    
    suite->results[suite->count] = result;
    
    // Se é o caso sequencial (1 thread), armazena o tempo de referência
    if (result.num_threads == 1) {
        suite->sequential_time = result.execution_time;
    }
    
    suite->count++;
}

// Calcula métricas de performance (speedup e eficiência)
void calculateMetrics(BenchmarkSuite* suite) {
    if (!suite || suite->sequential_time <= 0.0) return;
    
    for (int i = 0; i < suite->count; i++) {
        BenchmarkResult* result = &suite->results[i];
        
        // Speedup = T_sequential / T_parallel
        result->speedup = suite->sequential_time / result->execution_time;
        
        // Eficiência = Speedup / num_threads
        result->efficiency = result->speedup / result->num_threads;
    }
}

// Exporta resultados para arquivo CSV
void exportBenchmarkToCSV(BenchmarkSuite* suite, const char* filename) {
    if (!suite || !filename) return;
    
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: Could not create CSV file %s\n", filename);
        return;
    }
    
    // Cabeçalho do CSV
    fprintf(file, "Problem,Threads,Execution_Time_s,Tour_Length,Speedup,Efficiency,");
    fprintf(file, "Problem_Size,Iterations,Restarts,Timestamp\n");
    
    // Obter timestamp atual
    time_t now = time(NULL);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));
    
    // Dados
    for (int i = 0; i < suite->count; i++) {
        BenchmarkResult* result = &suite->results[i];
        
        fprintf(file, "%s,%d,%.6f,%.6f,%.4f,%.4f,",
                suite->problem_name,
                result->num_threads,
                result->execution_time,
                result->tour_length,
                result->speedup,
                result->efficiency);
        
        fprintf(file, "%d,%d,%d,%s\n",
                result->problem_size,
                result->num_iterations,
                result->num_restarts,
                timestamp);
    }
    
    fclose(file);
    printf("Benchmark results exported to: %s\n", filename);
}

// Destrói a suite de benchmark
void destroyBenchmarkSuite(BenchmarkSuite* suite) {
    if (suite) {
        if (suite->results) free(suite->results);
        if (suite->problem_name) free(suite->problem_name);
        free(suite);
    }
}

// Executa benchmark completo com diferentes números de threads
void runFullBenchmark(const char* input_file, const char* output_csv) {
    printf("Starting comprehensive TSP benchmark...\n");
    
    // Array de números de threads para testar
    int thread_counts[] = {1, 2, 4, 8, 16, 32, 64};
    int num_tests = sizeof(thread_counts) / sizeof(thread_counts[0]);
    int num_runs_per_test = 3; // Número de execuções por configuração para média
    
    // Lê parâmetros do arquivo
    FILE* inputFile = fopen(input_file, "r");
    if (!inputFile) {
        fprintf(stderr, "Error: Could not open input file %s\n", input_file);
        return;
    }
    
    char line[1024];
    int numIterations, numRestarts;
    unsigned int seed;
    
    if (fgets(line, sizeof(line), inputFile) == NULL || 
        sscanf(line, "%d %d %u", &numIterations, &numRestarts, &seed) != 3) {
        fprintf(stderr, "Error: Invalid input parameters\n");
        fclose(inputFile);
        return;
    }
    
    // Cria suite de benchmark
    char problem_name[256];
    snprintf(problem_name, sizeof(problem_name), "TSP_%s", input_file);
    BenchmarkSuite* suite = createBenchmarkSuite(problem_name);
    
    // Para cada configuração de threads
    for (int t = 0; t < num_tests; t++) {
        int num_threads = thread_counts[t];
        printf("Testing with %d thread(s)...\n", num_threads);
        
        double total_time = 0.0;
        double total_tour_length = 0.0;
        
        // Executa múltiplas vezes para obter média
        for (int run = 0; run < num_runs_per_test; run++) {
            // Reset do arquivo
            fseek(inputFile, 0, SEEK_SET);
            // Ignora resultado do fgets (corrige warning)
            if (fgets(line, sizeof(line), inputFile) == NULL) {
                fprintf(stderr, "Error reading file on run %d\n", run);
                continue;
            }
            
            // Cria solver
            TSPSolver* solver = createTSPSolver(seed + run);
            if (!solver) continue;
            
            // Carrega matriz
            loadAdjacencyMatrix(solver, inputFile);
            
            // Define número de threads
            omp_set_num_threads(num_threads);
            
            // Executa e mede tempo
            double start_time = omp_get_wtime();
            int* best_tour = solveTSP(solver, numIterations, numRestarts);
            double end_time = omp_get_wtime();
            
            if (best_tour) {
                double execution_time = end_time - start_time;
                double tour_length = calculateTourLength(solver, best_tour);
                
                total_time += execution_time;
                total_tour_length += tour_length;
                
                printf("  Run %d/%d: Time=%.4fs, Tour=%.2f\n", 
                       run + 1, num_runs_per_test, execution_time, tour_length);
                
                free(best_tour);
            }
            
            destroyTSPSolver(solver);
        }
        
        // Calcula médias e adiciona resultado
        if (num_runs_per_test > 0) {
            BenchmarkResult result = {
                .num_threads = num_threads,
                .execution_time = total_time / num_runs_per_test,
                .tour_length = total_tour_length / num_runs_per_test,
                .speedup = 0.0, // Será calculado depois
                .efficiency = 0.0, // Será calculado depois
                .problem_size = 0, // Será definido depois
                .num_iterations = numIterations,
                .num_restarts = numRestarts
            };
            
            addBenchmarkResult(suite, result);
        }
    }
    
    fclose(inputFile);
    
    // Obter tamanho do problema (número de cidades)
    TSPSolver* temp_solver = createTSPSolver(seed);
    if (temp_solver) {
        FILE* temp_file = fopen(input_file, "r");
        if (temp_file) {
            // Ignora resultado do fgets (corrige warning)
            if (fgets(line, sizeof(line), temp_file) != NULL) {
                loadAdjacencyMatrix(temp_solver, temp_file);
                
                // Atualiza tamanho do problema em todos os resultados
                for (int i = 0; i < suite->count; i++) {
                    suite->results[i].problem_size = temp_solver->matrixSize;
                }
            }
            fclose(temp_file);
        }
        destroyTSPSolver(temp_solver);
    }
    
    // Calcula métricas
    calculateMetrics(suite);
    
    // Exibe resumo
    printf("\n=== BENCHMARK SUMMARY ===\n");
    printf("Problem: %s\n", suite->problem_name);
    printf("Problem Size: %d cities\n", suite->results[0].problem_size);
    printf("Iterations: %d, Restarts: %d\n", numIterations, numRestarts);
    printf("Sequential Time: %.4f seconds\n", suite->sequential_time);
    printf("\nResults:\n");
    printf("Threads\tTime(s)\tSpeedup\tEfficiency\tTour Length\n");
    printf("-------\t-------\t-------\t----------\t-----------\n");
    
    for (int i = 0; i < suite->count; i++) {
        BenchmarkResult* r = &suite->results[i];
        printf("%d\t%.4f\t%.2fx\t%.2f%%\t\t%.2f\n",
               r->num_threads, r->execution_time, r->speedup, 
               r->efficiency * 100, r->tour_length);
    }
    
    // Exporta para CSV
    exportBenchmarkToCSV(suite, output_csv);
    
    // Análise de escalabilidade
    printf("\n=== SCALABILITY ANALYSIS ===\n");
    if (suite->count >= 2) {
        double best_speedup = 0.0;
        int best_threads = 1;
        
        for (int i = 0; i < suite->count; i++) {
            if (suite->results[i].speedup > best_speedup) {
                best_speedup = suite->results[i].speedup;
                best_threads = suite->results[i].num_threads;
            }
        }
        
        printf("Best speedup: %.2fx with %d threads\n", best_speedup, best_threads);
        printf("Theoretical maximum speedup: %dx\n", best_threads);
        printf("Parallel efficiency at best speedup: %.2f%%\n", 
               (best_speedup / best_threads) * 100);
        
        // Análise de escalabilidade forte
        printf("\nStrong Scaling Analysis:\n");
        for (int i = 1; i < suite->count; i++) {
            double scaling_efficiency = suite->results[i].speedup / suite->results[i].num_threads;
            printf("  %d threads: %.2f%% efficiency\n", 
                   suite->results[i].num_threads, scaling_efficiency * 100);
        }
    }
    
    destroyBenchmarkSuite(suite);
    printf("\nBenchmark completed successfully!\n");
}
#include "src/tspSolver.c"
#include "src/operations.c"
#include "src/tour.c"

// Função principal
int main(int argc, char* argv[]) {

    double startTime, endTime;

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
    
    // Resolve o problema do caixeiro viajante
    
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
    printf("Best tour found: ");
    for (int i = 0; i < solver->matrixSize; i++) {
        printf("%d ", bestTour[i]);
    }
    printf("\nTour length: %f\n", calculateTourLength(solver, bestTour));

    // Exibe o tempo de execução
    printf("Execution time: %f seconds\n", endTime - startTime);
    
    // Libera a memória
    free(bestTour);
    destroyTSPSolver(solver);
    
    return 0;
}
#ifndef TSPSOLVER_H
#define TSPSOLVER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

//Estrutura para o solucionador TSP
// Definição da estrutura para o solucionador TSP
typedef struct {
    double** adjacencyMatrix;
    int matrixSize;
    unsigned int seed;
} TSPSolver;

//Função para criar um novo solucionador TSP
TSPSolver* createTSPSolver(unsigned int seed);

//Função para destruir o solucionador TSP
void destroyTSPSolver(TSPSolver* solver);

//Função para carregar a matriz de adjacência do arquivo de entrada
void loadAdjacencyMatrix(TSPSolver* solver, FILE* inputFile);

//Função para definir o número de iterações e reinicializações
int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts);


#endif // TSPSOLVER_H
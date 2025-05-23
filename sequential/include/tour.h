#ifndef TOUR_H
#define TOUR_H
#include "tspSolver.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

//Função para resolver o problema do caixeiro viajante
int* solveTSP(TSPSolver* solver, int numIterations, int numRestarts);

//Função para calcular o comprimento do tour
double calculateTourLength(TSPSolver* solver, const int* tour);

//Função para gerar um tour aleatório
int* generateRandomTour(TSPSolver* solver);

//Função para realizar o shotgun hill climbing, ou seja, o algoritmo de escalada de colinas
// com reinicializações
int* shotgunHillClimbing(TSPSolver* solver, int numIterations, int numRestarts);

//Função para realizar o algoritmo hill climbing
void hillClimb(TSPSolver* solver, int numIterations, int** bestTourPtr, double* bestLengthPtr);


#endif // TOUR_H
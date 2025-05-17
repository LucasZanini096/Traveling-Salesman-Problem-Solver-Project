#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <float.h>

//Função para realizar a analise da matriz csv de um arquivo
double** parseCSVMatrix(FILE* inputFile, int* size);

//Função para verificar se a matriz é quadrada
int isSquareMatrix(double** matrix, int size);

//Função para realizar a função de troca
void swap(int* a, int* b);

//Função para embaralhar um array
void shuffleArray(int* array, int size, unsigned int* seed);

//Função que realiza a operação de reversão de uma sub-sequência de um array
void reverseSubArray(int* array, int start, int end);

// Realiza a operação 2-opt swap em um tour
void twoOptSwap(const int* tour, int* newTour, int i, int j, int tourSize);

#endif // OPERATIONS_H
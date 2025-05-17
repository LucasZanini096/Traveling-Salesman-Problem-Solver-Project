#ifndef RUN_H
#define RUN_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct {
    TSPSolver* solver;
    int numIterations;
    int numRestarts;
} TSPTestParams;


void* run_sequential_tsp(void* arg);

void* run_parallel_tsp(void* arg);


#endif // RUN_H
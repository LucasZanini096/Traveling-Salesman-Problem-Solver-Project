#ifndef PERFORMANCE_METRICS_H
#define PERFORMANCE_METRICS_H

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <time.h>

typedef struct {
    double sequential_time;  // Tempo de execução sequencial (em segundos)
    double parallel_time;    // Tempo de execução paralela (em segundos)
    int num_threads;         // Número de threads utilizadas
    double speedup;          // Valor do speedup calculado
    double efficiency;       // Valor da eficiência calculada
} PerformanceMetrics;


PerformanceMetrics run_performance_test(void* (*func_seq)(void*), void* (*func_par)(void*), 
                                        int num_threads, void* arg);

void print_metrics(PerformanceMetrics metrics);

#endif // PERFORMANCE_METRICS_H
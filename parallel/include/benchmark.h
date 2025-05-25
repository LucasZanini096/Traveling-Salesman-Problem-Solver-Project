#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

// Estrutura para armazenar resultados de benchmark
typedef struct {
    int num_threads;
    double execution_time;
    double tour_length;
    double speedup;
    double efficiency;
    int problem_size;
    int num_iterations;
    int num_restarts;
} BenchmarkResult;

// Estrutura para armazenar conjunto de resultados
typedef struct {
    BenchmarkResult* results;
    int count;
    int capacity;
    double sequential_time;
    char* problem_name;
} BenchmarkSuite;

// Funções para benchmark
BenchmarkSuite* createBenchmarkSuite(const char* problem_name);
void addBenchmarkResult(BenchmarkSuite* suite, BenchmarkResult result);
void calculateMetrics(BenchmarkSuite* suite);
void exportBenchmarkToCSV(BenchmarkSuite* suite, const char* filename);
void destroyBenchmarkSuite(BenchmarkSuite* suite);

// Função para executar benchmark completo
void runFullBenchmark(const char* input_file, const char* output_csv);

#endif // BENCHMARK_H
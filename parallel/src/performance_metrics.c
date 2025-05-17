#include "../include/performance_metrics.h"

void print_metrics(PerformanceMetrics metrics) {
    printf("Tempo sequencial: %.6f segundos\n", metrics.sequential_time);
    printf("Tempo paralelo (%d threads): %.6f segundos\n", metrics.num_threads, metrics.parallel_time);
    printf("Speedup: %.4f\n", metrics.speedup);
    printf("Eficiência: %.4f (%.2f%%)\n", metrics.efficiency, metrics.efficiency * 100);
}

/**
 * Executa um teste de desempenho comparando algoritmos sequencial e paralelo
 * 
 * @param func_seq Ponteiro para a função sequencial a ser testada
 * @param func_par Ponteiro para a função paralela a ser testada
 * @param num_threads Número de threads a serem utilizadas na execução paralela
 * @param arg Argumento a ser passado para as funções
 * @return Estrutura PerformanceMetrics com os resultados calculados
 */
PerformanceMetrics run_performance_test(void* (*func_seq)(void*), void* (*func_par)(void*), 
                                        int num_threads, void* arg) {
    PerformanceMetrics metrics;
    double start_time, end_time;
    
    // Configura o número de threads para 1 (execução sequencial)
    omp_set_num_threads(1);
    
    // Executa a versão sequencial e mede o tempo
    start_time = omp_get_wtime();
    func_seq(arg);
    end_time = omp_get_wtime();
    metrics.sequential_time = end_time - start_time;
    
    // Define o número de threads para a execução paralela
    omp_set_num_threads(num_threads);
    
    // Executa a versão paralela e mede o tempo
    start_time = omp_get_wtime();
    func_par(arg);
    end_time = omp_get_wtime();
    metrics.parallel_time = end_time - start_time;
    
    // Calcula as métricas
    metrics.num_threads = num_threads;
    metrics.speedup = metrics.sequential_time / metrics.parallel_time;
    metrics.efficiency = metrics.speedup / num_threads;
    
    return metrics;
}
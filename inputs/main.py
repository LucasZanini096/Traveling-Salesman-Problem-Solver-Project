#!/usr/bin/env python3
"""
Gerador de matrizes TSP para teste de escalabilidade
Cria matrizes simétricas de distâncias entre cidades para análise de performance
"""

import random
import sys
import argparse
import math

def generate_symmetric_matrix(size, min_dist=50, max_dist=999, seed=42):
    """
    Gera uma matriz TSP simétrica de tamanho especificado
    
    Args:
        size: Número de cidades (tamanho da matriz)
        min_dist: Distância mínima entre cidades
        max_dist: Distância máxima entre cidades  
        seed: Seed para reprodutibilidade
    
    Returns:
        Lista de listas representando a matriz de distâncias
    """
    random.seed(seed)
    
    # Inicializa matriz com zeros
    matrix = [[0 for _ in range(size)] for _ in range(size)]
    
    # Preenche matriz de forma simétrica
    for i in range(size):
        for j in range(i+1, size):
            # Gera distância aleatória
            distance = random.randint(min_dist, max_dist)
            matrix[i][j] = distance
            matrix[j][i] = distance  # Garantir simetria
    
    return matrix

def calculate_problem_complexity(size):
    """
    Calcula a complexidade computacional do problema
    """
    # Para hill climbing com 2-opt: O(n²) por iteração
    comparisons_per_iter = size * (size - 1) // 2
    
    return {
        'cities': size,
        'comparisons_per_iteration': comparisons_per_iter,
        'space_complexity': f'O({size}²)',
        'time_complexity_per_iter': f'O({size}²)'
    }

def write_tsp_file(filename, matrix, num_iterations=1000, num_restarts=100, seed=42):
    """
    Escreve arquivo de entrada TSP no formato esperado pelo programa
    """
    size = len(matrix)
    
    with open(filename, 'w') as f:
        # Cabeçalho: iterações, reinicializações, seed
        f.write(f"{num_iterations} {num_restarts} {seed}\n")
        
        # Matriz de distâncias
        for i, row in enumerate(matrix):
            line = ','.join(map(str, row))
            f.write(line + '\n')
    
    print(f"Arquivo TSP criado: {filename}")
    print(f"  - Cidades: {size}")
    print(f"  - Iterações: {num_iterations}")
    print(f"  - Reinicializações: {num_restarts}")

def create_scalability_test_suite():
    """
    Cria conjunto de arquivos para teste de escalabilidade forte
    """
    sizes = [8, 16, 32, 48, 64, 96, 128]
    base_seed = 42
    
    print("Criando suite de testes de escalabilidade...")
    print("=" * 50)
    
    for size in sizes:
        # Ajusta parâmetros baseado no tamanho do problema
        if size <= 16:
            iterations, restarts = 500, 50
        elif size <= 32:
            iterations, restarts = 1000, 100
        elif size <= 64:
            iterations, restarts = 1500, 150
        else:
            iterations, restarts = 2000, 200
            
        # Gera matriz
        matrix = generate_symmetric_matrix(size, seed=base_seed + size)
        
        # Nome do arquivo
        filename = f"input_{size}x{size}.txt"
        
        # Escreve arquivo
        write_tsp_file(filename, matrix, iterations, restarts, base_seed + size)
        
        # Mostra complexidade
        complexity = calculate_problem_complexity(size)
        print(f"  - Comparações por iteração: {complexity['comparisons_per_iteration']:,}")
        print()

def create_euclidean_matrix(size, width=1000, height=1000, seed=42):
    """
    Cria matriz baseada em distâncias euclidianas entre pontos aleatórios
    Mais realística que distâncias completamente aleatórias
    """
    random.seed(seed)
    
    # Gera coordenadas aleatórias para cada cidade
    cities = []
    for _ in range(size):
        x = random.uniform(0, width)
        y = random.uniform(0, height)
        cities.append((x, y))
    
    # Calcula matriz de distâncias euclidianas
    matrix = [[0 for _ in range(size)] for _ in range(size)]
    
    for i in range(size):
        for j in range(i+1, size):
            x1, y1 = cities[i]
            x2, y2 = cities[j]
            
            # Distância euclidiana
            dist = math.sqrt((x2 - x1)**2 + (y2 - y1)**2)
            dist = int(dist)  # Converte para inteiro
            
            matrix[i][j] = dist
            matrix[j][i] = dist
    
    return matrix, cities

def main():
    parser = argparse.ArgumentParser(
        description="Gerador de matrizes TSP para teste de escalabilidade"
    )
    
    parser.add_argument('--size', type=int, help='Tamanho da matriz (número de cidades)')
    parser.add_argument('--output', type=str, help='Nome do arquivo de saída')
    parser.add_argument('--iterations', type=int, default=1000, help='Número de iterações')
    parser.add_argument('--restarts', type=int, default=100, help='Número de reinicializações')
    parser.add_argument('--seed', type=int, default=42, help='Seed para geração aleatória')
    parser.add_argument('--euclidean', action='store_true', help='Usar distâncias euclidianas')
    parser.add_argument('--suite', action='store_true', help='Criar suite completa de testes')
    parser.add_argument('--min-dist', type=int, default=50, help='Distância mínima')
    parser.add_argument('--max-dist', type=int, default=999, help='Distância máxima')
    
    args = parser.parse_args()
    
    if args.suite:
        create_scalability_test_suite()
        return
    
    if not args.size:
        print("Error: --size é obrigatório (ou use --suite)")
        return
    
    if not args.output:
        args.output = f"input_{args.size}x{args.size}.txt"
    
    print(f"Gerando matriz TSP {args.size}x{args.size}...")
    
    if args.euclidean:
        matrix, cities = create_euclidean_matrix(args.size, seed=args.seed)
        print("Usando distâncias euclidianas entre pontos aleatórios")
    else:
        matrix = generate_symmetric_matrix(
            args.size, 
            min_dist=args.min_dist, 
            max_dist=args.max_dist, 
            seed=args.seed
        )
        print("Usando distâncias aleatórias uniformes")
    
    # Escreve arquivo
    write_tsp_file(args.output, matrix, args.iterations, args.restarts, args.seed)
    
    # Mostra estatísticas
    complexity = calculate_problem_complexity(args.size)
    print("\nEstatísticas do problema:")
    print(f"  - Cidades: {complexity['cities']}")
    print(f"  - Comparações por iteração: {complexity['comparisons_per_iteration']:,}")
    print(f"  - Complexidade espacial: {complexity['space_complexity']}")
    print(f"  - Complexidade temporal: {complexity['time_complexity_per_iter']}")
    
    # Análise de escalabilidade
    total_operations = complexity['comparisons_per_iteration'] * args.iterations * args.restarts
    print(f"  - Operações totais estimadas: {total_operations:,}")

if __name__ == "__main__":
    main()
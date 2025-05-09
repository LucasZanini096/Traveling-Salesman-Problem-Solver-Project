# Traveling Salesman Problem Solver

Lucas S. Rosa, Alfredo Goldman

The Traveling Salesman Problem (TSP) is a classic optimization challenge in computer science and operations research. It asks the question: "Given a list of cities and the distances between each pair of cities, what is the shortest possible route that visits each city exactly once and returns to the origin city?". This problem is of great importance in various real-world applications, including:

1. Logistics and transportation planning
2. Circuit board drilling in manufacturing
3. DNA sequencing in bioinformatics
4. Computer wiring
5. Delivery route optimization.

The TSP is known to be NP-hard, meaning that as the number of cities increases, the time required to find the optimal solution grows exponentially. This makes it an excellent candidate for exploring heuristic algorithms.

This solution implements a TSP solver using the Shotgun Hill Climbing algorithm. Here's a high-level overview of its functionality:

1. It reads a distance matrix from a CSV file, representing the distances between cities.
2. It uses a "shotgun" approach, which involves multiple restarts of a hill climbing search.
3. For each restart:
   • It generates a random initial tour.
   • It then repeatedly attempts to improve this tour using 2-opt swaps.
   • If no improvement is found, it moves to the next restart.
4. After all restarts, it returns the best tour found across all attempts.

The algorithm balances between exploration (through multiple random restarts) and exploitation (through hill climbing), aiming to find a good approximate solution to the TSP in a reasonable amount of time.

## Parallel Version of the Solution

```python
import numpy as np
import random
import sys
import multiprocessing as mp
from concurrent.futures import ProcessPoolExecutor
import time

# Function to generate a random tour
def generate_random_tour(n, random_state):
    tour = list(range(n))
    random_state.shuffle(tour)
    return tour

# Function to calculate the total length of a tour
def calculate_tour_length(tour, distance_matrix):
    tour_length = 0
    for i in range(len(tour)):
        tour_length += distance_matrix[tour[i]][tour[(i + 1) % len(tour)]]
    return tour_length

# Function to perform 2-opt swap
def two_opt_swap(tour, i, j):
    new_tour = tour.copy()
    # Reverse the segment between positions i and j (inclusive)
    new_tour[i:j+1] = reversed(new_tour[i:j+1])
    return new_tour

# Hill climbing function with 2-opt neighborhood
def hill_climbing(distance_matrix, max_iterations, seed):
    n = len(distance_matrix)
    random_state = random.Random(seed)
    
    # Generate random initial tour
    current_tour = generate_random_tour(n, random_state)
    current_length = calculate_tour_length(current_tour, distance_matrix)
    
    # Improvement iterations
    for _ in range(max_iterations):
        improved = False
        
        # Try all possible 2-opt swaps
        for i in range(n):
            if improved:
                break
            for j in range(i + 1, n):
                new_tour = two_opt_swap(current_tour, i, j)
                new_length = calculate_tour_length(new_tour, distance_matrix)
                
                if new_length < current_length:
                    current_tour = new_tour
                    current_length = new_length
                    improved = True
                    break
        
        # If no improvement found, stop
        if not improved:
            break
    
    return current_tour, current_length

# Shotgun hill climbing (worker function for parallel execution)
def shotgun_worker(args):
    distance_matrix, max_iterations, seed = args
    return hill_climbing(distance_matrix, max_iterations, seed)

# Main function
def main():
    # Read input
    line = sys.stdin.readline().strip().split()
    max_iterations = int(line[0])
    num_restarts = int(line[1])
    initial_seed = int(line[2])
    
    # Read distance matrix
    distance_matrix = []
    for line in sys.stdin:
        row = [float(x) for x in line.strip().split(',')]
        distance_matrix.append(row)
    
    # Prepare the arguments for parallel processing
    random_state = random.Random(initial_seed)
    seeds = [random_state.randint(0, 10000) for _ in range(num_restarts)]
    args_list = [(distance_matrix, max_iterations, seed) for seed in seeds]
    
    # Determine the number of processes to use (usually number of CPU cores)
    num_processes = min(mp.cpu_count(), num_restarts)
    
    # Run shotgun hill climbing in parallel
    best_tour = None
    best_length = float('inf')
    
    with ProcessPoolExecutor(max_workers=num_processes) as executor:
        results = list(executor.map(shotgun_worker, args_list))
        
        # Find the best tour among all restarts
        for tour, length in results:
            if length < best_length:
                best_tour = tour
                best_length = length
    
    # Output the results
    print("Best tour found:", " ".join(map(str, best_tour)))
    print(f"Tour length: {best_length}")

if __name__ == "__main__":
    main()
```

## Input
The first line of the input contains:
1. the number of iterations for each hill climbing attempt
2. the number of restarts (shotgun attempts)
3. a seed for the random number generator.

The following lines contains the distance matrix describing the distances of the cities. The input must be read from the standard input.

## Output
The program outputs:
1. The best tour found, represented as a sequence of city indices
2. The total length of this tour

In this output, the sequence of numbers represents the order in which cities are visited in the best tour found; and the tour length represents the total distance traveled in this tour. The output must be written to the standard output.

## Example
**Sample input 1**
```
2000 20 17
0.1,0.2,0.3
0.0,0.1,0.0
0.2,0.0,0.1
```

**Sample output 1**
```
Best tour found: 0 2 1
Tour length: 0.3
```
# Paralelização do algoritmo do Caxeiro Viajante

## Possíveis pontos para paralelização

1. Dentro da função hillClimb
2. Na função ShotgunHillClimb por meio da paralelização da execução do loop de restarts
3. Utilização do reducer dentro da função calculateTourLength

## Referências

1. Algoritmos de otimização: Hill Climbing e Simulated Annealing 
   1. https://medium.com/data-hackers/algoritmos-de-otimiza%C3%A7%C3%A3o-hill-climbing-e-simulated-annealing-3803061f66f0
2. Hill Climbing Algorithm: A Comprehensive Guide 
   1. https://medium.com/@tahsinsoyakk/hill-climbing-algorithm-a-comprehensive-guide-46e33f1ecc02
3. Programação Paralela com OpenMP: Modelo deTarefas
   1. https://books-sol.sbc.org.br/index.php/sbc/catalog/view/137/611/993
4. Programação Paralela com OpenMP: Modelo de Tarefas
   1. https://graduacao.mackenzie.br/pluginfile.php/698401/mod_resource/content/0/write-up.pdf
5. Paralelismo de tarefas utilizando OpenMP 4.5
   1. https://www.researchgate.net/publication/334138552_Paralelismo_de_tarefas_utilizando_OpenMP_45 
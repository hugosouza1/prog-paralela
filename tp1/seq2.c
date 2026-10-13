#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc, char** argv) {
    int rank, size;
    unsigned long long n = 0;
    double start_time, end_time;

    // Inicialização do MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (argc < 2) {
        if (rank == 0) printf("Uso: mpirun -np <processos> %s <N>\n", argv[0]);
        MPI_Finalize();
        return 1;
    }

    // 1. Mestre lê o N
    if (rank == 0) {
        n = strtoull(argv[1], NULL, 10);
    }
    
    // Difunde N para todos os processos
    MPI_Bcast(&n, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    // Sincroniza e inicia o relógio no Mestre (antes de dividir os dados)
    MPI_Barrier(MPI_COMM_WORLD);
    if (rank == 0) {
        start_time = MPI_Wtime();
    }

    // 2. Mestre descobre os primos até a raiz de N
    unsigned long long raiz_n = (unsigned long long)sqrt(n);
    char *primos_base = (char*)malloc((raiz_n + 1) * sizeof(char));
    memset(primos_base, 1, (raiz_n + 1) * sizeof(char));
    primos_base[0] = 0;
    primos_base[1] = 0;

    if (rank == 0) {
        for (unsigned long long p = 2; p * p <= raiz_n; ++p) {
            if (primos_base[p] == 1) {
                for (unsigned long long i = p * p; i <= raiz_n; i += p) {
                    primos_base[i] = 0;
                }
            }
        }
    }

    // 3. Broadcast do vetor base para todos os escravos
    MPI_Bcast(primos_base, raiz_n + 1, MPI_CHAR, 0, MPI_COMM_WORLD);

    // 4. Divisão de blocos (Fatiamento)
    // O domínio de busca é [2, N]. Quantidade total de números: N - 1
    unsigned long long total_elementos = n - 1;
    
    // Calcula início e fim da fatia de cada processo
    unsigned long long low_value = 2 + rank * (total_elementos) / size;
    unsigned long long high_value = 2 + (rank + 1) * (total_elementos) / size - 1;
    int local_size = high_value - low_value + 1;

    // Aloca e inicializa o bloco local do processo
    char *local_primos = (char*)malloc(local_size * sizeof(char));
    memset(local_primos, 1, local_size * sizeof(char));

    // 5. Crivo Local (Escravo)
    for (unsigned long long p = 2; p <= raiz_n; ++p) {
        if (primos_base[p] == 1) {
            // Acha o primeiro múltiplo de p dentro do bloco local
            unsigned long long primeiro_multiplo = (low_value % p == 0) ? low_value : low_value + (p - (low_value % p));
            
            // O próprio p não deve ser marcado como 0 se cair no bloco local
            if (primeiro_multiplo == p) {
                primeiro_multiplo += p;
            }

            // Invalida os múltiplos no array local
            for (unsigned long long j = primeiro_multiplo; j <= high_value; j += p) {
                local_primos[j - low_value] = 0;
            }
        }
    }

    // 6. Preparação para o Gather (Mestre reúne os blocos)
    int *recvcounts = NULL;
    int *displs = NULL;
    char *global_primos = NULL;

    if (rank == 0) {
        recvcounts = (int*)malloc(size * sizeof(int));
        displs = (int*)malloc(size * sizeof(int));
        global_primos = (char*)malloc(total_elementos * sizeof(char));

        for (int i = 0; i < size; i++) {
            unsigned long long l_low = 2 + i * (total_elementos) / size;
            unsigned long long l_high = 2 + (i + 1) * (total_elementos) / size - 1;
            recvcounts[i] = l_high - l_low + 1;
            displs[i] = (i == 0) ? 0 : displs[i - 1] + recvcounts[i - 1];
        }
    }

    // Envia o array de booleanos local de cada escravo para o global do Mestre
    MPI_Gatherv(local_primos, local_size, MPI_CHAR, global_primos, recvcounts, displs, MPI_CHAR, 0, MPI_COMM_WORLD);

    // Sincroniza e para o relógio (isolando escrita no arquivo, como o professor pediu)
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (rank == 0) {
        end_time = MPI_Wtime();
        printf("Tempo de processamento MPI: %lf segundos\n", end_time - start_time);

        // 7. Consolidação no arquivo primos.txt
        FILE *arquivo = fopen("primos.txt", "w");
        if (arquivo != NULL) {
            int impresso = 0;
            for (unsigned long long i = 0; i < total_elementos; i++) {
                if (global_primos[i] == 1) {
                    unsigned long long num_real = i + 2; // Ajuste do deslocamento (o array global começa no 2)
                    if (impresso == 0) {
                        fprintf(arquivo, "%llu", num_real);
                        impresso = 1;
                    } else {
                        fprintf(arquivo, ", %llu", num_real);
                    }
                }
            }
            fclose(arquivo);
            printf("Arquivo primos.txt gerado com sucesso.\n");
        } else {
            printf("Erro ao abrir primos.txt para escrita.\n");
        }
        
        free(recvcounts);
        free(displs);
        free(global_primos);
    }

    free(primos_base);
    free(local_primos);
    MPI_Finalize();
    return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mpi.h>

#define llu unsigned long long
#define MPI_LLU MPI_UNSIGNED_LONG_LONG



int main(int argc, char **argv) {
    MPI_Init(&argc, &argv);

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();


    if(argc < 2) {
        fprintf(stderr, "ERRO: valor N não informado por parâmetro.");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int log = (argc > 2 && strcmp(argv[2], "yes") == 0);


    llu n = strtoll(argv[1], &argv[1], 10);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size < 2) {
        fprintf(stderr, "ERRO: utilize pelo menos dois processos.");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    llu raizN = (llu) sqrt((double) n);
    while (raizN * raizN >= n) --raizN;               
    while ((raizN + 1) * (raizN + 1) < n) ++raizN;

    char *numeros = (char*)malloc(n * sizeof(char));
    llu* primosBase = (llu*)malloc(raizN * sizeof(llu));
    llu baseQtd = 0;

    if(numeros == NULL || primosBase == NULL){
        fprintf(stderr, "Falha alocação num/primos\n");
        exit(EXIT_FAILURE);
    } 

    
    
    if(rank == 0) {
        for(llu i = 2; i < n; i++) {
            numeros[i] = 'v';
        }
        numeros[0] = 0;
        numeros[1] = 0;
        for(llu i = 2; i <= raizN; i++) {
            if(numeros[i] == 'v') {
                primosBase[baseQtd++] = i;

                for(llu j = i*i; j <= raizN; j += i) {
                    numeros[j] = 0;
                }
            }
        }
    }

    MPI_Bcast(numeros, (int)n, MPI_CHAR, 0, MPI_COMM_WORLD);
    MPI_Bcast(&baseQtd, 1, MPI_LLU, 0, MPI_COMM_WORLD);
    MPI_Bcast(primosBase, (int)baseQtd, MPI_LLU, 0, MPI_COMM_WORLD);

    llu inicio = rank * (n / size);
    llu fim = (rank == size-1) ? n : (rank + 1) * (n / size);
    llu tamanho = fim - inicio;

    for(llu p = 0; p < baseQtd; ++p) {
        llu primo = primosBase[p];
        llu multiplo = ((inicio + primo - 1) / primo) * primo;

        if (multiplo < primo * primo) {
            multiplo = primo * primo;
        }

        for(llu j = multiplo; j < fim; j += primo) {
            numeros[j] = 0;
        }
    }

    llu *primosLocais = (llu*)malloc(tamanho * sizeof(llu));

    if(primosLocais == NULL) {
        fprintf(stderr, "Erro alocação primosLocas\n");
        exit(EXIT_FAILURE);
    }
    llu k = 0;

    for (llu i = inicio; i < fim; i++) {
        if (i >= 2 && numeros[i] == 'v') {
            primosLocais[k++] = i;
        }
    }

    if (rank != 0) {
        MPI_Send(&k, 1, MPI_LLU, 0, 0, MPI_COMM_WORLD);
        MPI_Send(primosLocais, (int)k, MPI_LLU, 0, 1, MPI_COMM_WORLD);

    } else {
        FILE *arquivo; 
        llu **resultados = (llu**)malloc(size * sizeof(llu*));
        llu *quantidades = (llu*)malloc(size * sizeof(llu));
        resultados[0] = primosLocais;
        quantidades[0] = k;
        for(int recebidos = 1; recebidos < size; recebidos++) {
            llu quantidade;
            MPI_Status status;

            MPI_Recv(
                &quantidade, 1, MPI_LLU,
                MPI_ANY_SOURCE, 0, MPI_COMM_WORLD,
                &status
            );

            int processo = status.MPI_SOURCE;

            quantidades[processo] = quantidade;
            resultados[processo] = malloc(quantidade * sizeof(llu));

            if (quantidade > 0 && resultados[processo] == NULL) {
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            MPI_Recv(
                resultados[processo], (int)quantidade, MPI_LLU,
                processo, 1, MPI_COMM_WORLD,
                MPI_STATUS_IGNORE
            );
        }

        if(log){
            arquivo = fopen("primos.txt", "w");
            
            if(arquivo == NULL) {
                perror("Erro ao abrir o arquivo");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            for(int i = 0; i < size; i++) {
              for(int j = 0; j < quantidades[i]; j++) {
                fprintf(arquivo, "%llu | \n", resultados[i][j]);
              }
            }
        }
        free(resultados);
        free(quantidades);
    }

    free(primosLocais);
    free(primosBase);
    free(numeros);

    
    double local = MPI_Wtime() - t0, total;
    MPI_Reduce(&local, &total, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) printf("n = %llu, size, %d, time %.6f\n", n, size, total);

    MPI_Finalize();
    return 0;
}
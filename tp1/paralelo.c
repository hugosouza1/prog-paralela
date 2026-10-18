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

    int log = (argc > 2 && strcmp(argv[2], "true") == 0);


    llu n = strtoull(argv[1], NULL, 10);

    if(n < 2) {
        fprintf(stderr, "ERRO: N deve ser pelo menos 2.");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

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

    llu inicio = rank * (n / size);
    llu fim = (rank == size-1) ? n : (rank + 1) * (n / size);
    llu tamanho = fim - inicio;

    char *numeros = (char*)malloc(tamanho + 1);
    llu* primosBase = (llu*)malloc((raizN + 1) * sizeof(llu));
    llu baseQtd = 0;
    
    if(numeros == NULL || primosBase == NULL){fprintf(stderr, "Falha alocação num/primos\n"); MPI_Abort(MPI_COMM_WORLD, 1);} 
    
    for(llu i = 0; i < tamanho; ++i) numeros[i] = 1;

    if(rank == 0) {
        char *crivoBase = (char*)malloc(raizN + 1);
        if(crivoBase == NULL){fprintf(stderr, "Falha alocação crivoBase\n"); MPI_Abort(MPI_COMM_WORLD, 1);}

        for(llu i = 0; i <= raizN; i++) crivoBase[i] = 1;
        crivoBase[0] = 0; 
        if(raizN >= 1) crivoBase[1] = 0;

        for(llu i = 2; i <= raizN; i++) {
            if(crivoBase[i]) {
                primosBase[baseQtd++] = i;

                for(llu j = i*i; j <= raizN; j += i) {
                    crivoBase[j] = 0;
                }
            }
        }

        free(crivoBase);
    }


    MPI_Bcast(&baseQtd, 1, MPI_LLU, 0, MPI_COMM_WORLD);
    MPI_Bcast(primosBase, (int)baseQtd, MPI_LLU, 0, MPI_COMM_WORLD);
    
    
    llu *primosLocais = (llu*)malloc((tamanho + 1) * sizeof(llu));
    if(primosLocais == NULL) {fprintf(stderr, "Erro alocação primosLocas\n"); MPI_Abort(MPI_COMM_WORLD, 1);}

    llu k = 0;
    
    for(llu p = 0; p < baseQtd; ++p) {
        llu primo = primosBase[p];
        llu multiplo = ((inicio + primo - 1) / primo) * primo;

        llu start = multiplo < primo * primo ? primo * primo : multiplo;
        for (llu j = start; j < fim; j += primo) numeros[j - inicio] = 0;
    }
    
    for (llu i = inicio; i < fim; i++) 
        if (i >= 2 && numeros[i - inicio]) 
            primosLocais[k++] = i;
        


    if (rank != 0) {
        MPI_Send(&k, 1, MPI_LLU, 0, 0, MPI_COMM_WORLD);
        MPI_Send(primosLocais, (int)k, MPI_LLU, 0, 1, MPI_COMM_WORLD);

        free(primosLocais);

    } else {
        FILE *arquivo; 
        llu **resultados = (llu**)malloc(size * sizeof(llu*));
        llu *quantidades = (llu*)malloc(size * sizeof(llu));
        resultados[0] = primosLocais; quantidades[0] = k;

        for(int recebidos = 1; recebidos < size; recebidos++) {
            llu quantidade;
            MPI_Status status;

            MPI_Recv(&quantidade, 1, MPI_LLU, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status);

            int processo = status.MPI_SOURCE;

            quantidades[processo] = quantidade;
            resultados[processo] = (llu*)malloc((quantidade + 1) * sizeof(llu));

            if (resultados[processo] == NULL) {
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            MPI_Recv(resultados[processo], (int)quantidade, MPI_LLU, processo, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        if(log){
            arquivo = fopen("primos.txt", "w");
            
            if(arquivo == NULL) {
                perror("Erro ao abrir o arquivo");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            for(int i = 0; i < size; i++) {
              for(llu j = 0; j < quantidades[i]; j++) {
                fprintf(arquivo, "%llu | ", resultados[i][j]);
              }
            }

            fclose(arquivo);
        }

        for(int f = 1; f < size; ++f) free(resultados[f]);
        free(primosLocais);
        free(resultados);
        free(quantidades);
    }

    free(primosBase);
    free(numeros);

    
    double local = MPI_Wtime() - t0, total;
    MPI_Reduce(&local, &total, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) printf("n = %llu, size, %d, time %.6f\n", n, size, total);

    MPI_Finalize();
    return 0;
}
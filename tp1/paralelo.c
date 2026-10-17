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
        if(log){
            arquivo = fopen("primos.txt", "w");
            
            if(arquivo == NULL) {
                perror("Erro ao abrir o arquivo");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
        }
        
        if(log)
            for(llu i = 0; i < k; i++) {
                fprintf(arquivo, "%llu | \n", primosLocais[i]);
            }

        for(int processo = 1; processo < size; processo++) {
            llu quantidade;
            MPI_Recv(&quantidade, 1, MPI_LLU, processo, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            llu *recebidos = (llu*)malloc(quantidade * sizeof(llu));
            if(recebidos == NULL) {fprintf(stderr, "Erro alocação recebidos\n"); exit(EXIT_FAILURE);}


            if (quantidade > 0 && recebidos == NULL && log) {
                fclose(arquivo);
                MPI_Abort(MPI_COMM_WORLD, 1);
            }

            MPI_Recv(recebidos, (int)quantidade, MPI_LLU, processo, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            if(log)
                for (llu i = 0; i < quantidade; i++) {
                    fprintf(arquivo, "%llu | \n", recebidos[i]);
                }

            free(recebidos);
        }
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
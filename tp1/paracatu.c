#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) fprintf(stderr, "use pelo menos 2 processos\n");
        MPI_Finalize();
        return 1;
    }

    unsigned long long n = 0;
    if (rank == 0) { scanf("%llu", &n); ++n; }   
    MPI_Bcast(&n, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    unsigned long long raiz = (unsigned long long) sqrt((double) n);
    while (raiz * raiz >= n) --raiz;               
    while ((raiz + 1) * (raiz + 1) < n) ++raiz;

    int nProc = size - 1;

    if (rank == 0) {
        int **matriz = malloc(sizeof(int*) * nProc);
        for (int w = 0; w < nProc; ++w) matriz[w] = malloc(sizeof(int) * (raiz + 1));
        int *proxIdx = calloc(nProc, sizeof(int));
        unsigned long long *carga = calloc(nProc, sizeof(unsigned long long));

        char *composto = calloc(raiz + 1, 1);     
        for (unsigned long long i = 2; i <= raiz; ++i) {
            if (composto[i]) continue;
            for (unsigned long long k = i * i; k <= raiz; k += i) composto[k] = 1;

            int menor = 0;
            for (int j = 1; j < nProc; ++j) if (carga[j] < carga[menor]) menor = j;
            carga[menor] += (n - raiz + i - 1) / i;
            matriz[menor][proxIdx[menor]++] = (int) i;
        }

        for (int w = 0; w < nProc; ++w) {
            MPI_Send(&proxIdx[w], 1, MPI_INT, w + 1, 0, MPI_COMM_WORLD);
            MPI_Send(matriz[w], proxIdx[w], MPI_INT, w + 1, 1, MPI_COMM_WORLD);
        }

        // recebe
        char *marcado = calloc(n, 1);
        for (int w = 0; w < nProc; ++w) {
            MPI_Status st;
            MPI_Probe(MPI_ANY_SOURCE, 2, MPI_COMM_WORLD, &st);

            int qtd; //limitacao da idade da pedra
            MPI_Get_count(&st, MPI_UNSIGNED_LONG_LONG, &qtd);

            unsigned long long *lista = malloc(sizeof(unsigned long long) * (qtd ? qtd : 1));
            MPI_Recv(lista, qtd, MPI_UNSIGNED_LONG_LONG, st.MPI_SOURCE, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (int j = 0; j < qtd; ++j) marcado[lista[j]] = 1;

            free(lista);
        }
        
        
        for (unsigned long long i = 2; i < n; ++i){
            if (!marcado[i]) printf("%llu | ", i);
        }

    } else {
        int idx;
        MPI_Recv(&idx, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int *minhaParte = malloc(sizeof(int) * (idx ? idx : 1));
        MPI_Recv(minhaParte, idx, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        char *copia = calloc(n, 1);
        unsigned long long cap = 512, qtd = 0;
        unsigned long long *unicos = malloc(sizeof(unsigned long long) * cap);

        for (int i = 0; i < idx; ++i) {
            unsigned long long p = minhaParte[i];
            for (unsigned long long k = p * p; k < n; k += p) {
                if (!copia[k]) {                            
                    copia[k] = 1;
                    if (qtd == cap) { cap *= 2; unicos = realloc(unicos, sizeof(unsigned long long) * cap); }
                    unicos[qtd++] = k;
                }
            }
        }

        MPI_Send(unicos, (int) qtd, MPI_UNSIGNED_LONG_LONG, 0, 2, MPI_COMM_WORLD);
        free(copia); free(unicos); free(minhaParte);
    }

    MPI_Finalize();
    return 0;
}
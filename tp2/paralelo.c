#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#define LLU long long int

typedef struct {
    LLU x;
    LLU y;
} Coordenadas;

typedef struct {
    LLU inicio;
    LLU fim;
} ThreadData;

LLU N_GEN, L, C, N;
int N_THREADS = 1;

pthread_barrier_t barreiraDeCalculo;

Coordenadas vizinhos[8] = {
    {1,  -1}, {1,  0}, {1,  1},
    {0,  -1},          {0,  1},
    {-1, -1}, {-1, 0}, {-1, 1}
};

char *anterior = NULL;
char *atual = NULL;

static inline int posicaoValida(LLU x, LLU y, LLU l, LLU c){
    return x >= 0 && x < l && y >= 0 && y < c;
}

void esperaBarreira(pthread_barrier_t *barreira){
    int erro = pthread_barrier_wait(barreira);
    if(erro != 0 && erro != PTHREAD_BARRIER_SERIAL_THREAD){
        fprintf(stderr, "Erro na barreira: %d\n", erro);
        exit(EXIT_FAILURE);
    }
}

void *processaBloco(void *arg){
    ThreadData *dados = (ThreadData*)arg;

    char *ant_local = anterior;
    char *atu_local = atual;

    LLU ini = dados->inicio;
    LLU fim = dados->fim;

    LLU linha = ini / C;
    LLU coluna = ini % C;

    for(LLU geracao = 0; geracao < N_GEN; ++geracao){
        for(LLU k = ini; k < fim; ++k){
            int vizinhosVivos = 0;

            for(int v = 0; v < 8; ++v){
                LLU x = linha + vizinhos[v].x;
                LLU y = coluna + vizinhos[v].y;

                if(posicaoValida(x, y, L, C)){
                    vizinhosVivos += ant_local[x * C + y];
                }
            }

            char celula = ant_local[k];

            if(celula){
                atu_local[k] = (vizinhosVivos == 2 || vizinhosVivos == 3);
            } else {
                atu_local[k] = (vizinhosVivos == 3);
            }

            coluna++;

            if(coluna == C){
                coluna = 0;
                linha++;
            }
        }

        esperaBarreira(&barreiraDeCalculo);

        char *aux = ant_local;
        ant_local = atu_local;
        atu_local = aux;

        linha = ini / C;
        coluna = ini % C;
    }

    return NULL;
}


// #define CACHE_LINE 64

// static char *alocaAlinhado(size_t n){
//     void *p = NULL;
//     if(posix_memalign(&p, CACHE_LINE, n) != 0) return NULL;
//     memset(p, 0, n);   // equivalente ao calloc (inclua <string.h>)
//     return (char*)p;
// }


int main(int argc, char *argv[]){

    struct timespec inicio_total, fim_total;
    struct timespec inicio_simulacao, fim_simulacao;

    clock_gettime(CLOCK_MONOTONIC, &inicio_total);

    if(argc < 2)
        { fprintf(stderr, "Quantidade de threads não foi informada.\n"); return EXIT_FAILURE;}

    N_THREADS = atoi(argv[1]);
    if(N_THREADS <= 0)
        { fprintf(stderr, "Quantidade de threads inválida.\n"); return EXIT_FAILURE;}

    if(scanf("%lld %lld %lld %lld", &N_GEN, &L, &C, &N) != 4)
        {fprintf(stderr, "Erro na leitura da entrada.\n"); return EXIT_FAILURE;}

    if(N_GEN < 0 || L <= 0 || C <= 0 || N < 0)
        {fprintf(stderr, "Entrada inválida.\n"); return EXIT_FAILURE;}

    Coordenadas *vivos = malloc(sizeof(Coordenadas) * N);
    if(N > 0 && vivos == NULL)
        {perror("Erro ao alocar coordenadas iniciais"); return EXIT_FAILURE;}

    for(LLU i = 0; i < N; ++i){
        LLU x, y;
        if(scanf("%lld %lld", &x, &y) != 2){
            fprintf(stderr, "Erro ao ler coordenadas na linha %lld.\n", i + 1);
            free(vivos);
            return EXIT_FAILURE;
        }
        if(x < 0 || x >= L || y < 0 || y >= C){
            fprintf(stderr, "Coordenada inválida na linha %lld.\n", i + 1);
            free(vivos);
            return EXIT_FAILURE;
        }
        vivos[i].x = x;
        vivos[i].y = y;
    }

    anterior = (char*)calloc(L * C, sizeof(char));
    atual    = (char*)calloc(L * C, sizeof(char));

    // anterior = alocaAlinhado(L * C);
    // atual    = alocaAlinhado(L * C);

    if(anterior == NULL || atual == NULL){
        perror("Erro ao alocar matrizes");
        free(vivos); free(anterior); free(atual);
        return EXIT_FAILURE;
    }

    for(LLU i = 0; i < N; ++i){
        anterior[vivos[i].x * C + vivos[i].y] = 1;
    }
    free(vivos);

    ThreadData *pacotes = (ThreadData*)malloc(N_THREADS * sizeof(ThreadData));
    pthread_t *threads  = (pthread_t*)malloc(N_THREADS * sizeof(pthread_t));

    if(pacotes == NULL || threads == NULL){
        perror("Erro ao alocar threads ou pacotes");
        free(pacotes); free(threads); free(anterior); free(atual);
        return EXIT_FAILURE;
    }


    
    LLU total = L * C;
    LLU blocos = total / N_THREADS;
    LLU resto = total % N_THREADS;

    for(int i = 0; i < N_THREADS; ++i){
        pacotes[i].inicio = i * blocos + (i < resto ? i : resto);
        pacotes[i].fim = pacotes[i].inicio + blocos + (i < resto ? 1 : 0);
    }


    // LLU total   = L * C;
    // LLU unidades = (total + CACHE_LINE - 1) / CACHE_LINE; 
    // LLU porThread = unidades / N_THREADS;
    // LLU resto     = unidades % N_THREADS;

    // for(int i = 0; i < N_THREADS; ++i){
    //     LLU uIni = i * porThread + (i < resto ? i : resto);
    //     LLU uFim = uIni + porThread + (i < resto ? 1 : 0);

    //     LLU ini = uIni * CACHE_LINE;
    //     LLU fim = uFim * CACHE_LINE;

    //     pacotes[i].inicio = ini < total ? ini : total;
    //     pacotes[i].fim    = fim < total ? fim : total;
    // }



    int erro = pthread_barrier_init(&barreiraDeCalculo, NULL, N_THREADS);
    if(erro != 0){
        fprintf(stderr, "Erro ao inicializar barreira: %d\n", erro);
        free(pacotes); free(threads); free(anterior); free(atual);
        return EXIT_FAILURE;
    }


    clock_gettime(CLOCK_MONOTONIC, &inicio_simulacao);

    for(int i = 0; i < N_THREADS; ++i){
        erro = pthread_create(&threads[i], NULL, processaBloco, &pacotes[i]);
        if(erro != 0){
            fprintf(stderr, "Erro ao criar thread %d: %d\n", i, erro); exit(EXIT_FAILURE);
        }
    }

    for(int i = 0; i < N_THREADS; ++i){
        erro = pthread_join(threads[i], NULL);
        if(erro != 0){
            fprintf(stderr, "Erro ao aguardar thread %d: %d\n", i, erro);
        }
    }

    clock_gettime(CLOCK_MONOTONIC, &fim_simulacao);

    double tempo_simulacao = (fim_simulacao.tv_sec - inicio_simulacao.tv_sec) + (fim_simulacao.tv_nsec - inicio_simulacao.tv_nsec) / 1e9;


    pthread_barrier_destroy(&barreiraDeCalculo);

    char *resultadoFinal = (N_GEN % 2 == 0) ? anterior : atual;

    LLU vivosFinais = 0;
    for(LLU k = 0; k < L * C; k++){
        if(resultadoFinal[k]) vivosFinais++;
    }

    printf("%lld\n%lld\n%lld\n%lld\n", N_GEN, L, C, vivosFinais);
    for(LLU i = 0; i < L; ++i){
        for(LLU j = 0; j < C; ++j){
            if(resultadoFinal[i * C + j]){
                printf("%lld %lld\n", i, j);
            }
        }
    }

    /* 
    // Saída visual 
    for(LLU i = 0; i < L; i++){
        for(LLU j = 0; j < C; j++){
            printf(resultadoFinal[i * C + j] ? "▨ " : "▢ ");
        }
        printf("\n");
    }
    */

    free(pacotes); free(threads); free(anterior); free(atual);

    clock_gettime(CLOCK_MONOTONIC, &fim_total);

    double tempo = (fim_total.tv_sec - inicio_total.tv_sec) + (fim_total.tv_nsec - inicio_total.tv_nsec) / 1e9;

    fprintf(stderr, "Tempo de execução: %.6f segundos\n", tempo);
    fprintf(stderr, "Tempo de simulacao: %.6f segundos\n", tempo_simulacao);

    return EXIT_SUCCESS;
}
#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define LLU long long int

typedef struct {
    LLU x;
    LLU y;
} Coordenadas;

typedef struct {
    LLU inicio;
    LLU fim;
    int id;
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

static inline int posicaoValida(LLU x, LLU y, LLU l, LLU c) {
    return x >= 0 && x < l && y >= 0 && y < c;
}

void esperaBarreira(pthread_barrier_t *barreira) {
    int erro = pthread_barrier_wait(barreira);
    if (erro != 0 && erro != PTHREAD_BARRIER_SERIAL_THREAD) {
        fprintf(stderr, "Erro na barreira: %d\n", erro);
        exit(EXIT_FAILURE);
    }
}

void *processaBloco(void *arg) {
    ThreadData *dados = (ThreadData*)arg;

    // Ponteiros locais na stack da thread (evita precisar da 2ª barreira!)
    char *ant_local = anterior;
    char *atu_local = atual;

    LLU ini = dados->inicio;
    LLU fim = dados->fim;

    for (LLU geracao = 0; geracao < N_GEN; geracao++) {
        // Dois laços evitam o custo pesado de divisão (k / C) e módulo (k % C)
        for (LLU i = ini; i < fim; i++) {
            LLU offset_linha = i * C;
            for (LLU j = 0; j < C; j++) {
                int vizinhosVivos = 0;

                for (int v = 0; v < 8; v++) {
                    LLU x = i + vizinhos[v].x;
                    LLU y = j + vizinhos[v].y;

                    if (posicaoValida(x, y, L, C)) {
                        vizinhosVivos += ant_local[x * C + y];
                    }
                }

                char celula = ant_local[offset_linha + j];
                if (celula) {
                    atu_local[offset_linha + j] = (vizinhosVivos == 2 || vizinhosVivos == 3) ? 1 : 0;
                } else {
                    atu_local[offset_linha + j] = (vizinhosVivos == 3) ? 1 : 0;
                }
            }
        }

        // Aguarda todas as threads terminarem de ler 'ant_local' e escrever em 'atu_local'
        esperaBarreira(&barreiraDeCalculo);

        // Cada thread troca seus próprios ponteiros locais instantaneamente
        char *aux = ant_local;
        ant_local = atu_local;
        atu_local = aux;
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Quantidade de threads não foi informada.\n");
        return EXIT_FAILURE;
    }

    N_THREADS = atoi(argv[1]);
    if (N_THREADS <= 0) {
        fprintf(stderr, "Quantidade de threads inválida.\n");
        return EXIT_FAILURE;
    }

    if (scanf("%lld %lld %lld %lld", &N_GEN, &L, &C, &N) != 4) {
        fprintf(stderr, "Erro na leitura da entrada.\n");
        return EXIT_FAILURE;
    }

    if (N_GEN < 0 || L <= 0 || C <= 0 || N < 0) {
        fprintf(stderr, "Entrada inválida.\n");
        return EXIT_FAILURE;
    }

    Coordenadas *vivos = malloc(sizeof(Coordenadas) * N);
    if (N > 0 && vivos == NULL) {
        perror("Erro ao alocar coordenadas iniciais");
        return EXIT_FAILURE;
    }

    for (LLU i = 0; i < N; i++) {
        LLU x, y;
        if (scanf("%lld %lld", &x, &y) != 2) {
            fprintf(stderr, "Erro ao ler coordenadas na linha %lld.\n", i + 1);
            free(vivos);
            return EXIT_FAILURE;
        }
        if (x < 0 || x >= L || y < 0 || y >= C) {
            fprintf(stderr, "Coordenada inválida na linha %lld.\n", i + 1);
            free(vivos);
            return EXIT_FAILURE;
        }
        vivos[i].x = x;
        vivos[i].y = y;
    }

    anterior = (char*)calloc(L * C, sizeof(char));
    atual    = (char*)calloc(L * C, sizeof(char));

    if (anterior == NULL || atual == NULL) {
        perror("Erro ao alocar matrizes");
        free(vivos);
        free(anterior);
        free(atual);
        return EXIT_FAILURE;
    }

    for (LLU i = 0; i < N; i++) {
        anterior[vivos[i].x * C + vivos[i].y] = 1;
    }
    free(vivos);

    // Garante que não criaremos mais threads do que linhas existentes
    if (N_THREADS > L) {
        N_THREADS = L;
    }

    ThreadData *pacotes = (ThreadData*)malloc(N_THREADS * sizeof(ThreadData));
    pthread_t *threads  = (pthread_t*)malloc(N_THREADS * sizeof(pthread_t));

    if (pacotes == NULL || threads == NULL) {
        perror("Erro ao alocar threads ou pacotes");
        free(pacotes);
        free(threads);
        free(anterior);
        free(atual);
        return EXIT_FAILURE;
    }

    LLU blocos = L / N_THREADS;
    LLU resto  = L % N_THREADS;

    for (int i = 0; i < N_THREADS; i++) {
        pacotes[i].inicio = i * blocos + (i < resto ? i : resto);
        pacotes[i].fim    = pacotes[i].inicio + blocos + (i < resto ? 1 : 0);
        pacotes[i].id     = i;
    }

    int erro = pthread_barrier_init(&barreiraDeCalculo, NULL, N_THREADS);
    if (erro != 0) {
        fprintf(stderr, "Erro ao inicializar barreira: %d\n", erro);
        free(pacotes);
        free(threads);
        free(anterior);
        free(atual);
        return EXIT_FAILURE;
    }

    for (int i = 0; i < N_THREADS; i++) {
        erro = pthread_create(&threads[i], NULL, processaBloco, &pacotes[i]);
        if (erro != 0) {
            fprintf(stderr, "Erro ao criar thread %d: %d\n", i, erro);
            exit(EXIT_FAILURE);
        }
    }

    for (int i = 0; i < N_THREADS; i++) {
        erro = pthread_join(threads[i], NULL);
        if (erro != 0) {
            fprintf(stderr, "Erro ao aguardar thread %d: %d\n", i, erro);
        }
    }

    pthread_barrier_destroy(&barreiraDeCalculo);

    // Se N_GEN for par, o estado final terminou em 'anterior'; se ímpar, em 'atual'
    char *resultadoFinal = (N_GEN % 2 == 0) ? anterior : atual;

    // Saída no formato exigido pela especificação do trabalho 
    LLU vivosFinais = 0;
    for (LLU k = 0; k < L * C; k++) {
        if (resultadoFinal[k]) vivosFinais++;
    }

    printf("%lld\n%lld\n%lld\n%lld\n", N_GEN, L, C, vivosFinais);
    for (LLU i = 0; i < L; i++) {
        for (LLU j = 0; j < C; j++) {
            if (resultadoFinal[i * C + j]) {
                printf("%lld %lld\n", i, j);
            }
        }
    }

    /* 
    // Saída visual 
    for (LLU i = 0; i < L; i++) {
        for (LLU j = 0; j < C; j++) {
            printf(resultadoFinal[i * C + j] ? "▨ " : "▢ ");
        }
        printf("\n");
    }
    */

    free(pacotes);
    free(threads);
    free(anterior);
    free(atual);

    return EXIT_SUCCESS;
}
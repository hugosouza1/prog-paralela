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
pthread_barrier_t barreiraDeTroca;

Coordenadas vizinhos[8] = {
	{1,  -1}, {1,  0}, {1,  1},
	{0,  -1},          {0,  1},
	{-1, -1}, {-1, 0}, {-1, 1}
};

char **anterior = NULL;
char **atual = NULL;

void freeMatriz(void *m, LLU linhas) {
	void **matriz = (void**)m;
	for(LLU i = 0; i < linhas; i++) {
		free(matriz[i]);
	}

	free(matriz);
}

int posicaoValida(long long int x, long long int y, long long int L, long long int C) {
	return x >= 0 && x < L && y >= 0 && y < C;
}

void esperaBarreira(pthread_barrier_t *barreira) {
	int erro = pthread_barrier_wait(barreira);

	if(erro != 0 && erro != PTHREAD_BARRIER_SERIAL_THREAD) {
		fprintf(stderr, "Erro na barreira: %d\n", erro);
		exit(EXIT_FAILURE);
	}
}

void *processaBloco(void *arg) {
	ThreadData *dados = (ThreadData*)arg;
	for(LLU geracao = 0; geracao < N_GEN; geracao++) {
		for(LLU i = dados->inicio; i < dados->fim; i++) {
			for(LLU j = 0; j < C; j++) {
				int vizinhosVivos = 0;

				for(int v = 0; v < 8; v++) {
					LLU x = i + vizinhos[v].x;
					LLU y = j + vizinhos[v].y;

					if(posicaoValida(x, y, L, C)) {
						if(anterior[x][y]) {
							vizinhosVivos++;
						}
					}
				}

				int celula = anterior[i][j];
				if(celula) {
					celula = (vizinhosVivos < 2 || vizinhosVivos > 3) ? 0 : 1;
				} else {
					celula = (vizinhosVivos == 3) ? 1 : 0;
				}
				atual[i][j] = celula;
			}
		}
		esperaBarreira(&barreiraDeCalculo);

		if(dados->id == 0) {
			char **aux = anterior;
			anterior = atual;
			atual = aux;

			// for(LLU i = 0; i < L; i++) {
			//   for(LLU j = 0; j < C; j++) {
			//     if(anterior[i][j]) {
			//       printf("▨ ");
			//     } else {
			//       printf("▢ ");
			//     }
			//   }
			//   printf("\n");
			// }
			// printf("\n");
		}
		esperaBarreira(&barreiraDeTroca);
	}

	return NULL;
}

int main(int argc, char *argv[]) {
	if(argc < 2) {
		fprintf(stderr, "Quantidade de threads não foi informado.\n");
		return EXIT_FAILURE;
	}

	N_THREADS = atoi(argv[1]);
	if(N_THREADS <= 0) {
		fprintf(stderr, "Quantidade de threads inválida.\n");
		return EXIT_FAILURE;
	}

	if(
		scanf("%lld", &N_GEN) != 1 ||
		scanf("%lld", &L) != 1 ||
		scanf("%lld", &C) != 1 ||
		scanf("%lld", &N) != 1
	) {
		fprintf(stderr, "Erro na leitura da entrada.\n");
		return EXIT_FAILURE;
	}

	if(N_GEN < 0 || L <= 0 || C <= 0 || N < 0) {
		fprintf(stderr, "Entrada inválida.\n");
		return EXIT_FAILURE;
	}

	Coordenadas *vivos = malloc(sizeof(Coordenadas) * N);
	if(N > 0 && vivos == NULL) {
		perror("Erro ao alocar coordenadas das células vivas inicialmente.");
		return EXIT_FAILURE;
	}

	for(LLU i = 0; i < N; i++) {
		LLU x, y;

		if(scanf("%lld %lld", &x, &y) != 2) {
			fprintf(stderr, "Erro ao ler coordenadas na linha %lld.\n", i+1);
			free(vivos);
			return EXIT_FAILURE;
		}

		if(x < 0 || x >= L || y < 0 || y >= C) {
			fprintf(stderr, "Coordenada inválida na linha %lld.\n", i+1);
			free(vivos);
			return EXIT_FAILURE;
		}

		vivos[i].x = x;
		vivos[i].y = y;
	}

	anterior = (char**)malloc(L * sizeof(char*));
	atual = (char**)malloc(L * sizeof(char*));

	if(anterior == NULL || atual == NULL) {
		perror("Erro ao alocar matrizes");
		free(vivos);
		freeMatriz(anterior, L);
		freeMatriz(atual, L);
	}

	for(LLU i = 0; i < L; i++) {
		anterior[i] = calloc(C, sizeof(char));
		atual[i] = calloc(C, sizeof(char));
	}

	for(LLU i = 0; i < N; i++) {
		anterior[vivos[i].x][vivos[i].y] = 1;
	}

	free(vivos);

	ThreadData *pacotes = (ThreadData*)malloc(N_THREADS * sizeof(ThreadData));
	pthread_t *threads = (pthread_t*)malloc(N_THREADS * sizeof(pthread_t));

	if(pacotes == NULL || threads == NULL) {
		perror("Erro ao alocar threads ou pacotes.\n");

		free(pacotes);
		free(threads);
		freeMatriz(anterior, L);
		freeMatriz(atual, L);

		return EXIT_FAILURE;
	}

	LLU blocos = L / N_THREADS;
	LLU resto = L % N_THREADS;

	for(int i = 0; i < N_THREADS; i++) {
		pacotes[i].inicio = i * blocos + (i < resto ? i : resto);
		pacotes[i].fim = pacotes[i].inicio + blocos + (i < resto ? 1 : 0);
		pacotes[i].id = i;
	}

	int erro;
	erro = pthread_barrier_init(&barreiraDeCalculo, NULL, N_THREADS);

	if(erro != 0) {
		fprintf(stderr, "Erro ao inicializar barreira: %d\n", erro);
		free(pacotes);
		free(threads);
		freeMatriz(anterior, L);
		freeMatriz(atual, L);

		return EXIT_FAILURE;
	}

	erro = pthread_barrier_init(&barreiraDeTroca, NULL, N_THREADS);

	if(erro != 0) {
		fprintf(stderr, "Erro ao inicializar barreira: %d\n", erro);
		free(pacotes);
		free(threads);
		freeMatriz(anterior, L);
		freeMatriz(atual, L);

		return EXIT_FAILURE;
	}

	for(int i = 0; i < N_THREADS; i++) {
		erro = pthread_create(&threads[i], NULL, processaBloco, &pacotes[i]);

		if(erro != 0) {
			fprintf(stderr, "Erro ao criar thread %d: %d\n", i, erro);
			exit(EXIT_FAILURE);
		}
	}

	for(int i = 0; i < N_THREADS; i++) {
		erro = pthread_join(threads[i], NULL);

		if(erro != 0) {
			fprintf(stderr, "Erro ao aguardar thread %d: %d\n", i, erro);
		}
	}

	pthread_barrier_destroy(&barreiraDeCalculo);
	pthread_barrier_destroy(&barreiraDeTroca);

	for(LLU i = 0; i < L; i++) {
		for(LLU j = 0; j < C; j++) {
			if(anterior[i][j]) {
				printf("▨ ");
			} else {
				printf("▢ ");
			}
		}
		printf("\n");
	}

	free(pacotes);
	free(threads);
	freeMatriz(anterior, L);
	freeMatriz(atual, L);

	return EXIT_SUCCESS;
}
#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define llu unsigned long long

double timestampAtual() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1e9;
}

void inicializa(char *numeros, llu n) {
    for (llu i = 0; i < n; ++i) numeros[i] = 'v';
}

void marcaMultiplos(char *numeros, llu n, llu i){
    for (llu k = i * i; k < n; k += i) {
        numeros[k] = 0;
    }

}

int crivo(char *numeros, llu n, llu **primos, llu *qtd) {
    for (llu i = 2; i * i < n; ++i) {
        if (numeros[i] == 'v') {
            marcaMultiplos(numeros, n, i);
        }
    }

    llu cap = 1024, k = 0;
    llu *v = (llu *)malloc(cap * sizeof(llu));
    if (v == NULL) return 1;

    for (llu i = 2; i < n; ++i) {
        if (numeros[i] == 'v') {
            if (k == cap) {
                cap *= 2;
                llu *tmp = (llu *)realloc(v, cap * sizeof(llu));
                if (tmp == NULL) { free(v); return 1; }
                v = tmp;
            }
            v[k++] = i;
        }
    }


    llu *exato = (llu *)realloc(v, (k ? k : 1) * sizeof(llu));
    if (exato != NULL) v = exato;

    *primos = v;
    *qtd = k;
    return 0;
}

int escrevePrimos(const llu *primos, llu qtd) {
    FILE *arquivo = fopen("primos.txt", "w");
    if (arquivo == NULL) return 1;

    for (llu i = 0; i < qtd; ++i) {
        fprintf(arquivo, "%llu\n", primos[i]);
    }
    fprintf(arquivo, "\n");
    fclose(arquivo);
    return 0;
}



int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: ./prog n log(opcional -> 'no' para desabilitar escrita)\n");
        return 1;
    }

    llu n = strtoull(argv[1], NULL, 10);

    if (n < 2) {
        fprintf(stderr, "ERRO: N deve ser pelo menos 2.\n");
        return 1;
    }

    int log = !(argc > 2 && strcmp(argv[2], "no") == 0);

    ++n;

    char *numeros = (char *)malloc(n * sizeof(char));

    if (numeros == NULL) {
        fprintf(stderr, "Erro de alocação numeros\n");
        return 1;
    }

    llu *primos = NULL;
    llu qtd = 0;

    // Medição: inicialização + crivo + identificação dos primos
    double t0 = timestampAtual();
    inicializa(numeros, n);
    if (crivo(numeros, n, &primos, &qtd) != 0) {
        fprintf(stderr, "Erro de alocação primos\n");
        free(numeros);
        return 1;
    }
    double t1 = timestampAtual();

    // Saída
    printf("n = %llu, tempo = %.6f\n", n - 1, t1 - t0);

    if (log && escrevePrimos(primos, qtd) != 0) {
        fprintf(stderr, "Erro na criação do arquivo!\n");
        free(primos);
        free(numeros);
        return 1;
    }

    free(primos);
    free(numeros);
    return 0;
}
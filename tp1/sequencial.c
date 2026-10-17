#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


double agora() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);

    return t.tv_sec + t.tv_nsec / 1e9;
}

void inicializa(char *numeros, unsigned long long n) {
    for (unsigned long long i = 0; i < n; ++i) numeros[i] = 'v';
}

void crivo(char *numeros, unsigned long long n) {
    for (unsigned long long i = 2; i * i < n; ++i) {
        if (numeros[i] == 'v') {
            for (unsigned long long k = i * i; k < n; k += i) {
                numeros[k] = 0;
            }
        }
    }
}

int escreve_primos(char *numeros, unsigned long long n) {
    FILE *arquivo = fopen("primos.txt", "w");
    if (arquivo == NULL) return 1;

    for (unsigned long long i = 2; i < n; ++i) {
        if (numeros[i] == 'v') fprintf(arquivo, "%llu | ", i);
    }
    fprintf(arquivo, "\n");
    fclose(arquivo);
    return 0;
}



int main(int argc, char **argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: ./prog n log");
        return 1;
    }

    unsigned long long n = strtoull(argv[1], NULL, 10);

    int log = (argc > 2 && strcmp(argv[2], "yes") == 0);

    ++n;

    char *numeros = (char *)malloc(n * sizeof(char));

    if (numeros == NULL) {
        fprintf(stderr, "Erro de alocação numeros\n");
        return 1;
    }

    // Medição: inicialização + crivo 
    double t0 = agora();
    inicializa(numeros, n);
    crivo(numeros, n);
    double t1 = agora();

    // Saída
    printf("n = %llu, tempo = %.6f\n", n - 1, t1 - t0);

    if (log && escreve_primos(numeros, n) != 0) {
        fprintf(stderr, "Erro na criação do arquivo!\n");
        free(numeros);
        return 1;
    }

    free(numeros);
    return 0;
}
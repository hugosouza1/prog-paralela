#include <stdio.h>
#include <stdlib.h>

// Uso: ./gerador <geracoes> <linhas> <colunas> <densidade 0..1> <semente>
int main(int argc, char *argv[]) {
    if (argc != 6) {
        fprintf(stderr, "Uso: %s geracoes linhas colunas densidade semente\n", argv[0]);
        return 1;
    }

    long long geracoes = atoll(argv[1]);
    long long linhas   = atoll(argv[2]);
    long long colunas  = atoll(argv[3]);
    double densidade   = atof(argv[4]);
    unsigned semente   = (unsigned) atoi(argv[5]);

    long long total = linhas * colunas;
    long long n = (long long)(total * densidade);

    if (linhas <= 0 || colunas <= 0 || densidade < 0 || densidade > 1) {
        fprintf(stderr, "Parametros invalidos.\n");
        return 1;
    }

    srand(semente);

    long long *posicoes = malloc(total * sizeof(long long));
    if (posicoes == NULL) return 1;

    for (long long i = 0; i < total; ++i) posicoes[i] = i;

    // Fisher-Yates parcial: só n passos, pois só precisamos das n primeiras posições
    for (long long i = 0; i < n; ++i) {
        long long restante = total - i;
        long long j = i + rand() % restante;
        long long tmp = posicoes[i]; posicoes[i] = posicoes[j]; posicoes[j] = tmp;
    }

    printf("%lld\n%lld\n%lld\n%lld\n", geracoes, linhas, colunas, n);
    for (long long i = 0; i < n; ++i)
        printf("%lld %lld\n", posicoes[i] / colunas, posicoes[i] % colunas);

    free(posicoes);
    return 0;
}
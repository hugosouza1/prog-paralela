
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
    srand(time(NULL));

    // Parametros da simulacao
    int geracoes = 10000;
    int linhas = 30;
    int colunas = 30;
    int n = 200;

    int total = linhas * colunas;

    if (n > total || n < 0) {
        fprintf(stderr, "Quantidade de celulas invalida.\n");
        return 1;
    }

    // Cria vetor com todas as posicoes
    int *posicoes = malloc(total * sizeof(int));

    if (posicoes == NULL) {
        return 1;
    }

    for (int i = 0; i < total; ++i) {
        posicoes[i] = i;
    }

    // Fisher-Yates: embaralhamento
    for (int i = total - 1; i > 0; --i) {
        int j = rand() % (i + 1);

        int temp = posicoes[i];
        posicoes[i] = posicoes[j];
        posicoes[j] = temp;
    }

    // Imprime entrada
    printf("%d\n", geracoes);
    printf("%d\n", linhas);
    printf("%d\n", colunas);
    printf("%d\n", n);

    // Converte posicao linear em coordenadas
    for (int i = 0; i < n; ++i) {
        int x = posicoes[i] / colunas;
        int y = posicoes[i] % colunas;

        printf("%d %d\n", x, y);
    }

    free(posicoes);

    return 0;
}

#define _XOPEN_SOURCE 600
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#define LLU long long int

typedef struct {
    LLU x;
    LLU y;
} Coordenadas;


static inline int posicaoValida(LLU x, LLU y, LLU l, LLU c){
    return x >= 0 && x < l && y >= 0 && y < c;
}

int contaVizinhos(char *matriz, LLU i, LLU j, LLU l, LLU c, Coordenadas vizinhanca[8]){
    int vizinhosVivos = 0;

    for(int v = 0; v < 8; ++v){
        LLU x = i + vizinhanca[v].x;
        LLU y = j + vizinhanca[v].y;

        if(posicaoValida(x, y, l, c)){
            vizinhosVivos += matriz[x * c + y];
        }
    }

    return vizinhosVivos;
}


char calculaCelula(char celula, int vizinhosVivos){
    if(celula){
        return (vizinhosVivos == 2 || vizinhosVivos == 3);
    }
    return vizinhosVivos == 3;
}


void processaGeracao(char *matrizSimulacao, char *matrizProxima, LLU l, LLU c, Coordenadas vizinhanca[8]){
    LLU i = 0;
    LLU j = 0;

    for(LLU k = 0; k < l * c; ++k){
        int vizinhosVivos = contaVizinhos(matrizSimulacao, i, j, l, c, vizinhanca);
        matrizProxima[k] = calculaCelula(matrizSimulacao[k], vizinhosVivos);

        ++j;
        if(j == c){
            j = 0;
            ++i;
        }
    }
}


void simula(char **matrizSimulacao, char **matrizProxima, LLU gen, LLU l, LLU c, Coordenadas vizinhanca[8]){
    for(LLU g = 0; g < gen; ++g){

        processaGeracao(*matrizSimulacao, *matrizProxima, l, c, vizinhanca);

        char *temp = *matrizSimulacao;
        *matrizSimulacao = *matrizProxima;
        *matrizProxima = temp;
    }
}


// Imprime o resultado final
void imprimeResultado(char *resultado, LLU gen, LLU l, LLU c){
    LLU vivosFinais = 0;

    for(LLU k = 0; k < l * c; ++k){
        if(resultado[k]){
            ++vivosFinais;
        }
    }

    printf("%lld\n%lld\n%lld\n%lld\n",
        gen, l, c, vivosFinais
    );

    for(LLU i = 0; i < l; ++i){
        for(LLU j = 0; j < c; ++j){
            if(resultado[i * c + j]){
                printf("%lld %lld\n", i, j);
            }
        }
    }
}


int main(){

    struct timespec inicio_total, fim_total;
    clock_gettime(CLOCK_MONOTONIC, &inicio_total);

    // Leitura
    LLU gen, l, c, n;
    if(scanf("%lld %lld %lld %lld", &gen, &l, &c, &n) != 4){
        return EXIT_FAILURE;
    }

    if(gen < 0 || l <= 0 || c <= 0 || n < 0){
        return EXIT_FAILURE;
    }
    
    Coordenadas *vivosIniciais = malloc(sizeof(Coordenadas) * n);

    if(n > 0 && vivosIniciais == NULL){
        fprintf(stderr, "Erro ao alocar coordenadas");
        return EXIT_FAILURE;
    }

    for(LLU i = 0; i < n; ++i){

        LLU a, b;
        if(scanf("%lld %lld", &a, &b) != 2){
            free(vivosIniciais); return EXIT_FAILURE;
        }

        if(a < 0 || a >= l || b < 0 || b >= c){
            free(vivosIniciais); return EXIT_FAILURE;
        }

        vivosIniciais[i].x = a;
        vivosIniciais[i].y = b;
    }

    // Alocação
    char *matrizSimulacao = calloc(l * c, sizeof(char));
    char *matrizProxima = calloc(l * c, sizeof(char));

    if(matrizSimulacao == NULL || matrizProxima == NULL){
        fprintf(stderr, "Erro ao alocar matrizes");
        free(vivosIniciais); free(matrizSimulacao); free(matrizProxima); 
        return EXIT_FAILURE;
    }

    // Inicialização da matriz
    for(LLU i = 0; i < n; ++i){
        matrizSimulacao[vivosIniciais[i].x * c + vivosIniciais[i].y] = 1;
    }

    free(vivosIniciais);

    // Vizinhança 
    Coordenadas vizinhanca[8] = {
        {-1, -1}, {-1,  0}, {-1,  1},
        { 0, -1},           { 0,  1},
        { 1, -1}, { 1,  0}, { 1,  1}
    };

    LLU genOriginal = gen;


    struct timespec inicio_simlacao, fim_simulacao;
    clock_gettime(CLOCK_MONOTONIC, &inicio_simlacao);

    simula(&matrizSimulacao, &matrizProxima, gen, l, c, vizinhanca);

    clock_gettime(CLOCK_MONOTONIC, &fim_simulacao);
    double tempo_simulacao = (fim_simulacao.tv_sec - inicio_simlacao.tv_sec) + (fim_simulacao.tv_nsec - inicio_simlacao.tv_nsec) / 1e9;
               
    // Imprime
    imprimeResultado(matrizSimulacao, genOriginal, l, c);
    
    clock_gettime(CLOCK_MONOTONIC, &fim_total);
    double tempo_total = (fim_total.tv_sec - inicio_total.tv_sec) + (fim_total.tv_nsec - inicio_total.tv_nsec) / 1e9;

    
    fprintf(stderr, "Tempo de simulacao: %.6f segundos\n", tempo_simulacao);
    fprintf(stderr, "Tempo de execução: %.6f segundos\n", tempo_total);


    free(matrizSimulacao); free(matrizProxima);
    return 0;
}

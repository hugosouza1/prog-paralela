#include <stdio.h>
#include <stdlib.h>


typedef struct {
    long long int x;
    long long int y;
} Coordenadas;


int posicaoValida(long long int x, long long int y, long long int l, long long int c){
    if(x >= 0 && x < l && y >= 0 && y < c)
        return 1;
    return 0;
}

int main(){
    long long int gen, l, c, n;
    scanf("%lld %lld %lld %lld", &gen, &l, &c, &n);
        
    Coordenadas *vivosIniciais = malloc (sizeof(Coordenadas) * n);

    for(int i = 0; i < n; ++i){
        long long int a,b;
        scanf("%lld %lld", &a, &b);
        vivosIniciais[i].x = a;
        vivosIniciais[i].y = b;
    }

    char *matrizSimulacao = calloc (l * c, sizeof(char));

    for(int i = 0; i < n; ++i){ matrizSimulacao[vivosIniciais[i].x * l + vivosIniciais[i].y] = 1; }
    
    Coordenadas vizinhanca[8] = {{-1, -1}, {-1, 0}, {-1, 1}, { 0,  1},{ 1,  1}, { 1, 0}, { 1, -1}, { 0, -1}};
    
    char *matrizProxima = calloc (l * c, sizeof(char));

    while(gen > 0){
        for(int k = 0; k < l*c; ++k){
            int i = k / c;
            int j = k % c;

            int vizinhosVivos = 0;

            for(int v = 0; v < 8; ++v){
                int x = i + vizinhanca[v].x;
                int y = j + vizinhanca[v].y;

                if(posicaoValida(x, y, l, c)){
                    if(matrizSimulacao[x * c + y]){
                        ++vizinhosVivos;
                    }
                }
            }

            int celula = matrizSimulacao[i * c + j];

            // Subpopulacao
            if(celula && vizinhosVivos < 2){
                celula = 0;
            }
            // Sobrevivencia
            else if(celula && (vizinhosVivos == 2 || vizinhosVivos == 3)){
                celula = 1;
            }
            // Superpopulacao
            else if(celula && vizinhosVivos > 3){
                celula = 0;
            }
            // Nascimento
            else if(!celula && vizinhosVivos == 3){
                celula = 1;
            }

            matrizProxima[i * c + j] = celula;
            
        }

        // Troca as matrizes
        char *temp = matrizSimulacao;
        matrizSimulacao = matrizProxima;
        matrizProxima = temp;

        // Debug
        // for(int i = 0; i < l; ++i){
        //     for(int j = 0; j < c; ++j){
        //         int val = matrizSimulacao[i][j];
        //         // ▢▣▨▬▩▦▧▤▰▱▢
        //         if(val){
        //             printf("▨ ");
        //         } else {
        //             printf("▢ ");
        //         }
        //     }
        //     printf("\n");
        // }

        // printf("\n");
        --gen;
    }

    for(int k = 0; k < l * c; ++k){
        int i = k / c;
        int j = k % c;

        int val = matrizSimulacao[i * c + j];

        if(val){
            printf("▨ ");
        } else {
            printf("▢ ");
        }

        if(j == c - 1){
            printf("\n");
        }
    }


    free(vivosIniciais);
    free(matrizSimulacao);
    free(matrizProxima);
}
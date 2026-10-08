#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int ehPrimo(int x){ 
    long long contador = 1;
    for(int i = 2; i < x; ++i){
        if(x % i == 0) contador++;
    }
    return contador == 1;
}

typedef struct {
    long long first; // valor
    int second; // validade
} Pair;

int main(){
    long long n = 0;
    scanf("%lld", &n);
    
    --n; // [2, n]

    Pair *numeros = (Pair*)malloc(n * sizeof(Pair));

    long long valor_atual = 2;
    for(int i = 0; i < n; ++i){
        numeros[i].first = valor_atual++;
        numeros[i].second = 1;
    }

    // ---------------------------------------------- //

    for(int i = 0; i < sqrt(n); ++i){
        if(numeros[i].second){
            if(ehPrimo(numeros[i].first)){
                for(int j = i+numeros[i].first; j < n; j+=numeros[i].first) numeros[j].second = 0;
            } else {
                numeros[i].second = 0;
            } 
        }
    }
                
    // ---------------------------------------------- //

    printf("\nNumeros primos: ");
    for(int i = 0; i < n; ++i){
        if(numeros[i].second){
            if(i==0)
                printf("%lld", numeros[i].first);
            else 
                printf(", %lld", numeros[i].first);
        }
    }
    printf("\n\n");


    free(numeros);
    return 0;
}
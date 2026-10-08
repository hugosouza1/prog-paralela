#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(){
    unsigned long long n = 0;
    scanf("%llu", &n);
    
    ++n;

    char *numeros = (char*)malloc((n) * sizeof(char));

    for(unsigned long long i = 0; i < n; ++i) numeros[i] = 'v';

    // ---------------------------------------------- //

    for(unsigned long long i = 2; i*i < n; ++i){
        if(numeros[i] == 'v'){
            for(unsigned long long k = i*i; k < n; k += i){
                numeros[k] = 0;
            }
        }
    }
                
    // ---------------------------------------------- //

    FILE *arquivo = fopen("primos.txt", "w");
    if(arquivo == NULL){
        printf("Erro na criação do arquivo!\n");
        free(numeros);
        return 1;
    }

    for(unsigned long long i = 2; i < n; ++i){
        if(numeros[i] == 'v'){
            if(i == 2 ) fprintf(arquivo, "%llu", i);
            else fprintf(arquivo, ", %llu", i);
        }
    }

    fclose(arquivo);


    free(numeros);
    return 0;
}
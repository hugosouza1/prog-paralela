#include <stdio.h>
#include <stdlib.h>
#include <math.h>


int main(){
    unsigned long long n = 0;
    scanf("%lld", &n);
    
    ++n;

    char *numeros = (char*)malloc((n) * sizeof(char));

    for(unsigned long long i = 0; i < n; ++i) numeros[i] = 'v';

    // ---------------------------------------------- //

    for(unsigned long long i = 2; i < sqrt(n); ++i){
        if(numeros[i] == 'v'){
            for(unsigned long long k = i+i; k < n; k += i){
                numeros[k] = 0;
            }
        }
    }
                
    // ---------------------------------------------- //

    printf("\nNumeros primos: ");
    for(unsigned long long i = 2; i < n; ++i){
        if(numeros[i] == 'v'){
            if(i==2)
                printf("%lld", i);
            else 
                printf(", %lld", i);
        }
    }
    printf("\n\n");


    free(numeros);
    return 0;
}
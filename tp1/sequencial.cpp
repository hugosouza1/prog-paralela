#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

using namespace std;


bool ehPrimo(int x){ 
    long long contador = 1;
    for(int i = 2; i < x; ++i){
        if(x % i == 0) contador++;
    }
    // cout << x << "-"<< contador << " ";
    return contador == 1;
}

int main(){
    long long n;
    std::cin >> n;
    --n;

    vector<pair<long long, bool>> numeros(n);

    long long valor_atual = 2;

    generate(numeros.begin(), numeros.end(), [&valor_atual](){
        return make_pair(valor_atual++, true);
    });

    for(int i = 0; i < numeros.size(); ++i){
        if(numeros[i].second)
            if(ehPrimo(numeros[i].first)){
                for(int j = i; j < numeros.size(); j+=numeros[i].first){
                    numeros[j].second = false;
                }
                numeros[i].second = true;
            } else {
                numeros[i].second = false;
            }
    }
    
    for(auto [x, valido] : numeros){
        if(valido) cout << x << " ";
    }

    cout << "\n";
}
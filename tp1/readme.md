# Crivo de Eratóstenes: sequencial e paralelo (MPI)

## Compilação

```bash
gcc -O2 sequencial.c -o sequencial.exe -lm
mpicc -O2 paralelo.c -o paralelo.exe -lm
```

## Execução

Argumentos: `N` (maior número testado, mínimo 2) e, opcionalmente, `no` para **não** gravar `primos.txt`.

```bash
./sequencial.exe 100000000 no
mpirun -np 4 ./paralelo.exe 100000000 no     # mínimo 2 processos
```

## Saída

```
n = 100000000, tempo = 1.234567                 (sequencial)
n = 100000000, size = 4, tempo = 0.654321       (paralelo)
```

O tempo é de parede e não inclui a escrita em arquivo. No paralelo, é o maior tempo entre os processos e inclui a coleta dos primos pelo processo 0.

## Observações

- O vetor usa 1 byte por número: N = 10⁹ ocupa ~1 GB. Se faltar memória, o processo é encerrado com `Killed`.
- Use no máximo um processo por núcleo físico; acima disso é preciso `--oversubscribe`.
- Rode em uma pasta do Linux (não em `/mnt/c`) para evitar lentidão e interferência do OneDrive nas medições.
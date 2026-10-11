#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(int argc, char **argv) {
  if(argc < 2) {
    printf("ERRO: valor N não informado por parâmetro.");
  }

  unsigned long long n = strtoll(argv[1], &argv[1], 10);

  MPI_Init(&argc, &argv);
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int shmid;
  char *numeros = NULL;
  key_t key = IPC_PRIVATE;
  
  if(rank == 0) {
    shmid = shmget(key, n, IPC_CREAT | 0600);
    if(shmid < 0) {
      perror("Erro ao criar espaço de memória");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }
  
  MPI_Bcast(&shmid, 1, MPI_INT, 0, MPI_COMM_WORLD);
  numeros = (char*)shmat(shmid, NULL, 0);
  if(numeros == (char*)-1) {
    perror("Erro no shmat");
    MPI_Abort(MPI_COMM_WORLD, 1);
  }
  
  if(rank == 0) {
    for(unsigned long long i = 2; i < n; i++) {
      numeros[i] = 'v';
    }
    for(unsigned long long i = 2; i*i < n; i++) {
      if(numeros[i] == 'v') {
        for(unsigned long long j = i*i; j < n; j += i) {
          numeros[j] = 0;
        }
      }
    }
  }

  MPI_Barrier(MPI_COMM_WORLD);

  unsigned long long porcao = n / size;
  unsigned long long inicio = porcao * rank;
  unsigned long long fim = (rank == size - 1) ? n : (inicio + porcao);
  int tamanho = fim - inicio;
  unsigned long long *primos = (unsigned long long*)malloc((size_t)tamanho * sizeof(unsigned long long));

  unsigned long long k = 0;
  for(unsigned long long i = inicio; i < fim; i++) {
    if(numeros[i] == 'v') {
      primos[k] = i;
      k++;
    }
  }

  unsigned long long* quantidades = NULL;

  if(rank == 0) {
    quantidades = (unsigned long long*)malloc(size * sizeof(unsigned long long));
  }

  MPI_Gather(&k, 1, MPI_UNSIGNED_LONG_LONG, quantidades, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

  unsigned long long* todosOsPrimos = NULL;
  int* deslocamentos = NULL;
  int* contagens = NULL;
  unsigned long long tamanhoTotal = 0;
  if(rank == 0) {
    deslocamentos = (int*)malloc(size * sizeof(int));
    contagens = (int*)malloc(size * sizeof(int));
    for(int i = 0; i < size; i++) {
      deslocamentos[i] = tamanhoTotal;
      contagens[i] = (int)quantidades[i];
      tamanhoTotal += quantidades[i];
    }
    todosOsPrimos = (unsigned long long*)malloc(tamanhoTotal * sizeof(unsigned long long));
  }

  MPI_Gatherv(primos, k, MPI_UNSIGNED_LONG_LONG, todosOsPrimos, contagens, deslocamentos, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    FILE *arquivo = fopen("primos.txt", "w");

    if(arquivo == NULL) {
      printf("Erro na criação do arquivo!\n");
    } else {
      for (unsigned long long i = 0; i < tamanhoTotal; i++) {
        if(i == 0) {
          fprintf(arquivo, "%llu", todosOsPrimos[i]);
        }
        else {
          fprintf(arquivo, ", %llu", todosOsPrimos[i]);
        }
      }
      fclose(arquivo);
    }

    free(quantidades);
    free(contagens);
    free(deslocamentos);
    free(todosOsPrimos);
}

  shmdt(numeros);
  if(rank == 0) {
    shmctl(shmid, IPC_RMID, NULL);
  }

  MPI_Finalize();


  return 0;
}
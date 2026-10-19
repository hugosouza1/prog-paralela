CC = gcc
CFLAGS = -O2 -pg -fno-inline -fno-inline-functions-called-once 
LDFLAGS = -lm

e?=10
size?=4

log=true

.PHONY: sequencial paralelo

all:
	gcc -O2 sequencial.c -o sequencial.exe $(LDFLAGS)
	mpicc -O2 paralelo.c -o paralelo.exe -lm


sequencial:
	$(CC) $(CFLAGS) $@.c -o $@.exe $(LDFLAGS)
	./$@.exe $(e) $(log)
	gprof -b -l $@.exe gmon.out > analise.txt

clean:
	rm -f *.exe gmon.out analise.txt primos.txt

paralelo:
	mpicc -O2 $@.c -o $@.exe -lm
	mpirun -np $(size) ./$@.exe $(e) $(log)
CC = gcc
CFLAGS = -O0 -pg -fno-inline -fno-inline-functions-called-once 
LDFLAGS = -lm

T?=10
size?=4

log=true

.PHONY: sequencial paralelo

sequencial:
	$(CC) $(CFLAGS) $@.c -o $@.exe $(LDFLAGS)
	./$@.exe $(T) $(log)
	gprof -b -l $@.exe gmon.out > analise.txt

clean:
	rm -f *.exe gmon.out analise.txt primos.txt

paralelo:
	mpicc -O0 $@.c -o $@.exe -lm
	mpirun -np $(size) ./$@.exe $(T) $(log)
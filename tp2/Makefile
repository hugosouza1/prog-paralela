CC = gcc
CFLAGS = -O2 -pg -fno-inline -fno-inline-functions-called-once 

e?=entrada_media.txt
size?=4

log=no

.PHONY: sequencial paralelo

all:
	gcc -O2 sequencial.c -o sequencial.exe 
	gcc -pthread -O2 paralelo.c -o paralelo.exe 


sequencial:
	$(CC) $(CFLAGS) $@.c -o $@.exe 
	./$@.exe $(e) $(log)
	gprof -b -l $@.exe gmon.out > analise.txt

clean:
	rm -f *.exe gmon.out analise.txt primos.txt

paralelo:
	gcc -pthread -O2 $@.c -o $@.exe -lm
	./$@.exe $(size) $(e) $(log)
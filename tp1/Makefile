CC = gcc
CFLAGS = -O0 -pg -Wall -Wextra
LDFLAGS = -lm

POGRAMA = sequencial
ENTRADA ?= 100

all:
	$(CC) $(CFLAGS) $(POGRAMA).c -o $(POGRAMA) $(LDFLAGS)
	echo $(ENTRADA) | ./$(POGRAMA)
	gprof -b -p $(POGRAMA) gmon.out > analise.txt

clean:
	rm -f $(POGRAMA) gmon.out analise.txt
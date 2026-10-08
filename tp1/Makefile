CC = gcc
CFLAGS = -O0 -pg -Wall -Wextra
LDFLAGS = -lm

POGRAMA = sequencial

all:
	$(CC) $(CFLAGS) $(POGRAMA).c -o $(POGRAMA) $(LDFLAGS)
	./$(POGRAMA)
	gprof $(POGRAMA) gmon.out > analise.txt

clean:
	rm -f $(POGRAMA) gmon.out analise.txt
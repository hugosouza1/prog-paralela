gcc -pg -O0 sequencial.c -o programa -lm
./programa
gprof -p programa gmon.out > analise.txt
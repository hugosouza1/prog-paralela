gcc -pg -O0 sequencial.c -o programa -lm
echo 100000000 | ./programa
gprof -p programa gmon.out > analise.txt
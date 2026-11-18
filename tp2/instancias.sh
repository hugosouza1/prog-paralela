gcc -O2 -pg -fno-inline -fno-inline-functions-called-once sequencial.c -o sequencial.exe && \

./sequencial.exe instancias/g100_gen10000_d0.30_s1.txt no && \
gprof -b sequencial.exe gmon.out > analise100.txt && \

./sequencial.exe instancias/g1000_gen200_d0.30_s1.txt no && \
gprof -b sequencial.exe gmon.out > analise1000.txt && \

./sequencial.exe instancias/g5000_gen20_d0.30_s1.txt no && \
gprof -b sequencial.exe gmon.out > analise5000.txt
   gcc -O2 -pg -fno-inline -fno-inline-functions-called-once -fno-tree-loop-distribute-patterns sequencial.c -o sequencial_prof.exe -lm
   for n in 10000000 100000000 1000000000; do
     ./sequencial_prof.exe $n no
     gprof -b -p sequencial_prof.exe gmon.out > flat_$n.txt
   done
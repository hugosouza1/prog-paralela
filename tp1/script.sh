rm -f tempos_seq.txt tempos_par.txt
for n in 100000 1000000 10000000 100000000 1000000000; do
  for rep in 1 2 3 4 5; do
    ./sequencial.exe $n no >> tempos_seq.txt
    for p in 2 3 4; do
      mpirun -np $p ./paralelo.exe $n no >> tempos_par.txt
    done
  done
done
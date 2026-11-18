#!/usr/bin/env bash
# Roda os experimentos do Jogo da Vida (sequencial e paralelo) e grava um CSV.
#
# Compile antes, sem -pg e com a mesma otimização nas duas versões:
gcc -O2 -o sequencial sequencial.c
gcc -O2 -pthread -o paralelo paralelo.c
gcc -O2 -o gerador geradorEntrada.c
#
# Uso: chmod +x rodar_experimentos.sh && ./rodar_experimentos.sh

rm resultados.csv
# rm -rf instancias 

set -euo pipefail
export LC_ALL=C   # garante ponto decimal na leitura dos tempos

# ---------- Configuração ----------
SEQ=./sequencial
PAR=./paralelo
GER=./gerador
SAIDA=resultados.csv
DIR_INST=instancias

TAMANHOS=(100 1000 5000)
DENSIDADES=(0.30 0.30 0.30)
GERACOES=(10000 200 20)

THREADS=(1 2 3 4 5 6 7 8 12 16)
SEEDS=(1)
REPS=10
# -----------------------------------

mkdir -p "$DIR_INST"
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

if [ ! -f "$SAIDA" ]; then
    echo "versao,threads,grade,geracoes,densidade,seed,rep,tempo_sim,tempo_total,correto" > "$SAIDA"
fi

declare -A REF   # hash da saída do sequencial, por (grade, seed)

# roda <versao> <threads> <binario>   (usa as variáveis globais do laço)
roda() {
    local versao=$1 threads=$2 bin=$3
    local out="$TMP/out.txt" err="$TMP/err.txt"
    local extra=(no) ok="NA" hash key

    # 1ª repetição: escreve a saída para validar
    if [ "$rep" -eq 1 ]; then extra=(); fi

    if [ "$versao" = "seq" ]; then
        "$bin" "$inst" "${extra[@]}" > "$out" 2> "$err"
    else
        "$bin" "$inst" "$threads" "${extra[@]}" > "$out" 2> "$err"
    fi

    local sim tot
    sim=$(awk '/simulacao/ {print $4}' "$err")
    tot=$(awk '/execu/ {print $4}' "$err")

    if [ "$rep" -eq 1 ]; then
        hash=$(md5sum < "$out" | cut -d' ' -f1)
        key="${L}_${seed}"
        if [ -z "${REF[$key]:-}" ]; then REF[$key]=$hash; fi
        if [ "$hash" = "${REF[$key]}" ]; then ok=1; else ok=0; fi
    fi

    echo "$versao,$threads,$L,$G,$D,$seed,$rep,$sim,$tot,$ok" >> "$SAIDA"
    echo "[$(date +%H:%M:%S)] $versao t=$threads grade=$L rep=$rep sim=${sim}s ok=$ok" >&2
}

# Repetição no laço externo: as configurações ficam intercaladas no tempo,
# o que evita que aquecimento/throttling enviesem uma configuração só.
for rep in $(seq 1 "$REPS"); do
    for i in "${!TAMANHOS[@]}"; do
        L=${TAMANHOS[$i]}
        D=${DENSIDADES[$i]}
        G=${GERACOES[$i]}

        for seed in "${SEEDS[@]}"; do
            inst="$DIR_INST/g${L}_gen${G}_d${D}_s${seed}.txt"
            if [ ! -f "$inst" ]; then
                "$GER" "$G" "$L" "$L" "$D" "$seed" > "$inst"
            fi

            roda seq 1 "$SEQ"
            for t in "${THREADS[@]}"; do
                roda par "$t" "$PAR"
            done
        done
    done
done

echo "Pronto: $SAIDA" >&2
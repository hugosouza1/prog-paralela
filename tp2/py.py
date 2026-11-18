#!/usr/bin/env python3
"""Lê o resultados.csv do rodar_experimentos.sh e gera os gráficos.

Uso:
    python3 gerar_graficos.py [resultados.csv] [pasta_saida]

Requer: pandas e matplotlib (pip install pandas matplotlib)

Gráficos gerados (um PNG e um PDF cada):
    1. Tempo de simulação por nº de threads
    2. Speedup (simulação) por nº de threads, com a linha ideal S(N) = N
    3. Eficiência paralela por nº de threads
    4. Tempo total por nº de threads
    5. Speedup total (inclui leitura/alocação seriais; mostra a Lei de Amdahl)

Metodologia:
    - Baseline = versão SEQUENCIAL (não o paralelo com 1 thread).
    - Speedup = média do sequencial na instância / tempo de cada execução paralela.
    - Pontos = média; barras de erro = desvio padrão entre repetições.
    - correto: 1 = validado, NA = não validado (repetições 2..N, rodadas com "no"),
      0 = saída diferente do sequencial -> o script aborta.
    - A repetição 1 escreve a saída, então seu tempo_total é descartado.
      tempo_sim vale em todas as repetições.
"""

import os
import sys

import matplotlib

matplotlib.use("Agg")  # funciona sem display (WSL, servidor)
import matplotlib.pyplot as plt
import pandas as pd

from matplotlib.ticker import FuncFormatter, LogLocator, NullFormatter

# ---------- Configuração ----------
NUCLEOS_FISICOS = 4   # acima disso, as threads usam SMT (hyper-threading)
DPI = 200
# -----------------------------------


def carregar(caminho):
    df = pd.read_csv(caminho)

    # Só 0 é divergência real; NA = repetição não validada (esperado)
    ruins = df[df["correto"] == 0]
    if len(ruins) > 0:
        sys.exit("ERRO: saída diferente do sequencial:\n"
                 + ruins.to_string(index=False))

    # A repetição 1 inclui a escrita da saída: tempo_total não é comparável
    df.loc[df["rep"] == 1, "tempo_total"] = float("nan")

    return df


def calcular(df):
    seq = df[df["versao"] == "seq"]
    par = df[df["versao"] == "par"].copy()

    if seq.empty or par.empty:
        sys.exit("O CSV precisa ter execuções 'seq' e 'par'.")

    # Baseline por instância (grade, seed); mean ignora NaN do tempo_total
    base = (seq.groupby(["grade", "seed"])
               .agg(seq_sim=("tempo_sim", "mean"),
                    seq_total=("tempo_total", "mean"))
               .reset_index())
    par = par.merge(base, on=["grade", "seed"], how="left")

    par["speedup"] = par["seq_sim"] / par["tempo_sim"]
    par["eficiencia"] = par["speedup"] / par["threads"]
    par["speedup_total"] = par["seq_total"] / par["tempo_total"]

    resumo = (par.groupby(["grade", "threads"])
                 .agg(sim_m=("tempo_sim", "mean"), sim_s=("tempo_sim", "std"),
                      tot_m=("tempo_total", "mean"), tot_s=("tempo_total", "std"),
                      sp_m=("speedup", "mean"), sp_s=("speedup", "std"),
                      ef_m=("eficiencia", "mean"), ef_s=("eficiencia", "std"),
                      spt_m=("speedup_total", "mean"),
                      spt_s=("speedup_total", "std"),
                      n=("tempo_sim", "size"),
                      n_tot=("tempo_total", "count"))
                 .reset_index())

    if (resumo["n"] < 2).any():
        print("AVISO: há configurações com menos de 2 repetições; "
              "o desvio padrão não é confiável.", file=sys.stderr)

    # std = NaN quando n < 2: troca por 0 só para a barra de erro não falhar
    cols_std = [c for c in resumo.columns if c.endswith("_s")]
    resumo[cols_std] = resumo[cols_std].fillna(0.0)

    ref_seq = (seq.groupby("grade")
                  .agg(sim_m=("tempo_sim", "mean"), sim_s=("tempo_sim", "std"),
                       tot_m=("tempo_total", "mean"), tot_s=("tempo_total", "std"),
                       n=("tempo_sim", "size"),
                       n_tot=("tempo_total", "count")))
    return resumo, ref_seq


def grafico(resumo, col, titulo, ylabel, arquivo, pasta,
            ref=None, ideal=False, hline1=False, logy=False):
    """col: prefixo da coluna (sim, tot, sp, ef, spt).
    ref: dict grade -> valor do sequencial, desenhado como linha pontilhada."""
    grades = sorted(resumo["grade"].unique())
    cores = {g: f"C{i}" for i, g in enumerate(grades)}
    threads = sorted(resumo["threads"].unique())

    fig, ax = plt.subplots(figsize=(8, 5))

    for g in grades:
        d = resumo[resumo["grade"] == g].sort_values("threads")
        ax.errorbar(d["threads"], d[f"{col}_m"], yerr=d[f"{col}_s"],
                    marker="o", capsize=3, color=cores[g],
                    label=f"{g}×{g}")
        if ref is not None and g in ref:
            ax.axhline(ref[g], color=cores[g], linestyle=":", alpha=0.7)

    if ideal:
        ax.plot(threads, threads, "k--", label="Ideal  S(N) = N")
    if hline1:
        ax.axhline(1.0, color="k", linestyle="--", label="Ideal  E(N) = 1")

    # Fronteira núcleos físicos / SMT
    if max(threads) > NUCLEOS_FISICOS:
        ax.axvline(NUCLEOS_FISICOS, color="gray", linestyle="--", alpha=0.8)
        ax.axvspan(NUCLEOS_FISICOS, max(threads) + 0.5,
                   color="gray", alpha=0.10)
        ax.text(NUCLEOS_FISICOS + 0.1, 0.98,
                "SMT (threads lógicas)", transform=ax.get_xaxis_transform(),
                va="top", fontsize=9, color="dimgray")

    if logy:
        ax.set_yscale("log")
        ax.yaxis.set_major_locator(
            LogLocator(base=10, subs=(1, 1.5, 2, 3, 4, 5, 7), numticks=30))
        ax.yaxis.set_major_formatter(FuncFormatter(lambda v, _: f"{v:g}"))
        ax.yaxis.set_minor_formatter(NullFormatter())
        
        
    ax.set_xticks(threads)
    ax.set_xlabel("Quantidade de threads")
    ax.set_ylabel(ylabel)
    ax.set_title(titulo)
    ax.grid(True, alpha=0.3)
    ax.legend(title="Grade")
    if ref is not None:
        ax.text(0.01, 0.01, "Linhas pontilhadas: versão sequencial",
                transform=ax.transAxes, fontsize=8, color="dimgray")

    fig.tight_layout()
    for ext in ("png", "pdf"):
        fig.savefig(os.path.join(pasta, f"{arquivo}.{ext}"), dpi=DPI)
    plt.close(fig)


def main():
    caminho = sys.argv[1] if len(sys.argv) > 1 else "resultados.csv"
    pasta = sys.argv[2] if len(sys.argv) > 2 else "graficos"
    os.makedirs(pasta, exist_ok=True)

    df = carregar(caminho)
    resumo, ref_seq = calcular(df)

    resumo.to_csv(os.path.join(pasta, "resumo.csv"), index=False)
    ref_seq.to_csv(os.path.join(pasta, "resumo_sequencial.csv"))

    grafico(resumo, "sim",
            "Tempo de simulação por quantidade de threads",
            "Tempo de simulação (s)", "grafico1_tempo_simulacao", pasta,
            ref=ref_seq["sim_m"].to_dict(), logy=True)

    grafico(resumo, "sp",
            "Speedup por quantidade de threads",
            "Speedup  S(N) = T_seq / T_par(N)", "grafico2_speedup", pasta,
            ideal=True)

    grafico(resumo, "ef",
            "Eficiência paralela por quantidade de threads",
            "Eficiência  E(N) = S(N) / N", "grafico3_eficiencia", pasta,
            hline1=True)

    grafico(resumo, "tot",
            "Tempo total por quantidade de threads",
            "Tempo total (s)", "grafico4_tempo_total", pasta,
            ref=ref_seq["tot_m"].to_dict(), logy=True)

    grafico(resumo, "spt",
            "Speedup total (inclui leitura e alocação seriais)",
            "Speedup total  T_seq / T_par(N)", "grafico5_speedup_total", pasta,
            ideal=True)

    print(f"Gráficos e tabelas-resumo salvos em '{pasta}/'")


if __name__ == "__main__":
    main()
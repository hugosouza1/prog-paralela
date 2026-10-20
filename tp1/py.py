"""
Gera os gráficos da análise de desempenho do Crivo de Eratóstenes.

Entradas (uma medição por linha, várias repetições por configuração):
  tempos_seq.txt : "N tempo"        (saída do ./crivo N)
  tempos_par.txt : "N p tempo"      (saída do mpirun -np p ./paralelo.exe N)

Uso:  python3 analise.py
Requer: pip install pandas matplotlib numpy
"""
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# ---------- 1. Perfil do gprof (copie os valores do seu flat profile) ----------
PERFIL = {
    "marcaMultiplos": 85,
    "inicializa": 15,
    # "escreve_primos": 9.01,
}

# ---------- 2. Leitura e agregação ----------
seq = pd.read_csv("tempos_seq.txt", sep=r"\s+", names=["n", "t"])
par = pd.read_csv("tempos_par.txt", sep=r"\s+", names=["n", "p", "t"])

seq_m = seq.groupby("n")["t"].agg(["mean", "std"]).reset_index()
par_m = par.groupby(["n", "p"])["t"].agg(["mean", "std"]).reset_index()

# Speedup e eficiência: T_seq(N) / T_par(N, p)
tab = par_m.rename(columns={"mean": "t_par", "std": "std_par"}).merge(
    seq_m.rename(columns={"mean": "t_seq", "std": "std_seq"}), on="n"
)
tab["speedup"] = tab["t_seq"] / tab["t_par"]
tab["eficiencia"] = tab["speedup"] / tab["p"]

tab = tab[["n", "p", "t_seq", "std_seq", "t_par", "std_par", "speedup", "eficiencia"]]
tab.to_csv("resultados.csv", index=False, float_format="%.4f")
print(tab.to_string(index=False))

# ---------- 3. Gráfico: perfil do gprof ----------
fig, ax = plt.subplots(figsize=(6, 3.5))
nomes, vals = list(PERFIL.keys()), list(PERFIL.values())
ax.barh(nomes[::-1], vals[::-1])
for i, v in enumerate(vals[::-1]):
    ax.text(v + 0.8, i, f"{v:.1f}%", va="center")
ax.set_xlabel("% do tempo total (gprof)")
ax.set_title("Perfil da versão sequencial")
ax.set_xlim(0, 100)
fig.tight_layout()
fig.savefig("grafico_perfil.png", dpi=150)

# ---------- 4. Gráfico: tempo x N (crescimento) ----------
fig, ax = plt.subplots(figsize=(7, 4.5))
ax.errorbar(seq_m["n"], seq_m["mean"], yerr=seq_m["std"], marker="o", label="Sequencial")
for p, g in par_m.groupby("p"):
    ax.errorbar(g["n"], g["mean"], yerr=g["std"], marker="s", label=f"MPI p={p}")

# Curva teórica O(N log log N), escalada para coincidir com o último ponto sequencial
n_ref = seq_m["n"].to_numpy(dtype=float)
teor = n_ref * np.log(np.log(n_ref))
teor *= seq_m["mean"].iloc[-1] / teor[-1]
ax.plot(n_ref, teor, "k--", label=r"$O(N\log\log N)$ (escalada)")

ax.set_xscale("log")
ax.set_yscale("log")
ax.set_xlabel("N")
ax.set_ylabel("Tempo (s)")
ax.set_title("Crescimento do tempo de execução")
ax.legend()
ax.grid(True, which="both", alpha=0.3)
fig.tight_layout()
fig.savefig("grafico_tempo_n.png", dpi=150)

# ---------- 5. Gráficos: speedup e eficiência x p ----------
for col, ylabel, arq, ideal in [
    ("speedup", "Speedup", "grafico_speedup.png", True),
    ("eficiencia", "Eficiência", "grafico_eficiencia.png", False),
]:
    fig, ax = plt.subplots(figsize=(6.5, 4.5))
    for n, g in tab.groupby("n"):
        g = g.sort_values("p")
        ax.plot(g["p"], g[col], marker="o", label=f"N = {int(n):,}".replace(",", "."))
    ps = np.sort(tab["p"].unique())
    if ideal:
        ax.plot(ps, ps, "k--", label="Ideal")
    else:
        ax.axhline(1.0, color="k", linestyle="--", label="Ideal")
    ax.axhline(0, color="gray", linewidth=0.5)
    ax.set_xticks(ps)
    ax.set_xlabel("Número de processos (p)")
    ax.set_ylabel(ylabel)
    ax.legend()
    ax.grid(True, alpha=0.3)
    fig.tight_layout()
    fig.savefig(arq, dpi=150)

print("\nGerados: grafico_perfil.png, grafico_tempo_n.png, "
      "grafico_speedup.png, grafico_eficiencia.png, resultados.csv")
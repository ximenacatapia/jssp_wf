#!/usr/bin/env python3
"""
Genera un diagrama de Gantt y gráfica de recorrido de maquinas a partir de
una instancia JSSP y un archivo de solucion producido por jssp-wfa.

Uso:
    python3 tools/gantt.py <instancia.txt> <solucion.sol.txt> [salida.png]

El script recupera los tiempos de inicio reejecutando el calendario (cada
operacion empieza cuando terminan su predecesora de trabajo y la operacion
anterior en su maquina), y dibuja las barras.
"""
import sys
import re
import matplotlib

matplotlib.use("Agg")  
import matplotlib.pyplot as plt
from matplotlib import colormaps


def leer_instancia(path):
    """Devuelve (n, m, jobs) donde jobs[j] = [(maquina, duracion), ...]."""
    with open(path) as f:
        toks = f.read().split()
    it = iter(toks)
    n = int(next(it))
    m = int(next(it))
    next(it)  # cota superior del archivo (no la usa)
    next(it)  # cota inferior del archivo
    jobs = []
    for _ in range(n):
        ops = []
        for _ in range(m):
            maq = int(next(it))
            dur = int(next(it))
            ops.append((maq, dur))
        jobs.append(ops)
    return n, m, jobs


def leer_solucion(path):
    """Devuelve orden[maquina] = [(job, pos, dur), ...] en orden de la maquina."""
    orden = []
    with open(path) as f:
        for linea in f:
            linea = linea.strip()
            if not linea or not linea.startswith("["):
                continue
            # extrae cada tripleta [a, b, c]
            trips = re.findall(r"\[\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\]", linea)
            if trips:
                orden.append([(int(a), int(b), int(c)) for a, b, c in trips])
    return orden


 """
    Recalcula el tiempo de inicio de cada operacion (job, pos).
    Replica la logica del calendario
"""
def reconstruir_tiempos(n, m, jobs, orden):
    start, end = {}, {}
    # fin de la ultima operacion colocada en cada maquina / trabajo
    fin_maquina = [0] * m
    fin_trabajo = [0] * n
    pendiente = [0] * m  # indice de la siguiente op a colocar en cada maquina

    # Colocamos operaciones respetando el orden de cada maquina, iterando hasta
    total = n * m
    colocadas = 0
    guard = 0
    while colocadas < total:
        progreso = False
        for k in range(m):
            if pendiente[k] >= len(orden[k]):
                continue
            job, pos, dur = orden[k][pendiente[k]]

            if pos > 0 and (job, pos - 1) not in end:
                continue
            ini_trabajo = end[(job, pos - 1)] if pos > 0 else 0
            ini = max(fin_maquina[k], ini_trabajo)
            start[(job, pos)] = ini
            end[(job, pos)] = ini + dur
            fin_maquina[k] = ini + dur
            pendiente[k] += 1
            colocadas += 1
            progreso = True
        guard += 1
        if not progreso:
            raise RuntimeError(
                "No se pudo reconstruir el calendario (posible deadlock en la "
                "solucion). Operaciones colocadas: %d/%d" % (colocadas, total)
            )
        if guard > total + 5:
            break
    return start, end


def dibujar_gantt(n, m, jobs, orden, start, end, salida):
    makespan = max(end.values())
    fig, ax = plt.subplots(figsize=(14, 8))
    cmap = colormaps.get_cmap("hsv")

    for k in range(m):
        for (job, pos, dur) in orden[k]:
            ini = start[(job, pos)]
            ax.barh(
                y=k,
                width=dur,
                left=ini,
                height=0.8,
                color=cmap((job % n) / max(n, 1)),
                edgecolor="white",
                linewidth=0.3,
            )

    ax.set_xlabel("Tiempo")
    ax.set_ylabel("Maquina")
    ax.set_title("Diagrama de Gantt  (makespan = %d)" % makespan)
    ax.set_ylim(-0.5, m - 0.5)
    ax.set_yticks(range(m))
    ax.invert_yaxis()  # maquina 0 arriba
    ax.set_xlim(0, makespan)
    fig.tight_layout()
    fig.savefig(salida, dpi=130)
    print("Gantt guardado en: %s   (makespan=%d)" % (salida, makespan))


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        sys.exit(1)
    inst_path = sys.argv[1]
    sol_path = sys.argv[2]
    salida = sys.argv[3] if len(sys.argv) > 3 else "gantt.png"

    n, m, jobs = leer_instancia(inst_path)
    orden = leer_solucion(sol_path)
    if len(orden) != m:
        print(
            "Aviso: la solucion tiene %d maquinas y la instancia %d."
            % (len(orden), m)
        )
    start, end = reconstruir_tiempos(n, m, jobs, orden)
    dibujar_gantt(n, m, jobs, orden, start, end, salida)


if __name__ == "__main__":
    main()
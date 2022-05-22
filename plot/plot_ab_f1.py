import csv
import math

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['H2-Simpfer', 'SA-Simpfer', 'SAH']
markers = ['o', '^', '.']
colors = ['blue', 'red', 'purple']

k_vals = [1, 5, 10, 20, 30, 40, 50]

plt.rcParams.update({
    "font.size": 10,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})


def read_csv_f1(fname):
    min_f1 = 100
    f1_scores = {}
    for alg in ['SAP', 'H2P', 'SAH']:
        f1_scores[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            f1 = float(row[4])
            f1_scores[alg].append(f1)
            if f1 < min_f1:
                min_f1 = f1
    return f1_scores, min_f1


def draw_f1_a(dataset, f1_scores, y_min, y_max, y_interval):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset, fontsize=12)
    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel(r'F1-Score (\%)', fontsize=11)
    ax.set_yticks(range(math.floor(y_min), 101, y_interval))
    ax.set_ylim(y_min, y_max)
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.plot(k_vals, f1_scores['H2P'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, f1_scores['SAP'], color=colors[2], linewidth=1.5, marker=markers[2], ms=9, mew=1.5)
    ax.plot(k_vals, f1_scores['SAH'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)

    figs.tight_layout(rect=[0.135, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_ablation_f1.pdf' % dataset, format='pdf')


if __name__ == "__main__":
    t, min_t = read_csv_f1("results/Automotive/ablation.tsv")
    print(datasets[0], min_t)
    draw_f1_a(datasets[0], t, 92, 100.5, 2)

    t, min_t = read_csv_f1("results/CDs/ablation.tsv")
    print(datasets[1], min_t)
    draw_f1_a(datasets[1], t, 60, 102, 10)

    t, min_t = read_csv_f1("results/MovieLens/ablation.tsv")
    print(datasets[2], min_t)
    draw_f1_a(datasets[2], t, 95, 100.3, 1)

    t, min_t = read_csv_f1("results/Music100/ablation.tsv")
    print(datasets[3], min_t)
    draw_f1_a(datasets[3], t, 85, 101, 3)

    t, min_t = read_csv_f1("results/Netflix/ablation.tsv")
    print(datasets[4], min_t)
    draw_f1_a(datasets[4], t, 95, 100.3, 1)

import csv
import math

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix', 'DEEP', 'Gist', 'GloVe', 'ImageNet', 'Msong']
algorithms = ['H2-ALSH', 'SA-ALSH']
markers = ['o', '^']
colors = ['blue', 'red']

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
    for alg in ['H2-ALSH', 'SA-ALSH']:
        f1_scores[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            f1 = float(row[3])
            f1_scores[alg].append(f1)
            if f1 < min_f1:
                min_f1 = f1
    return f1_scores, min_f1


def draw_f1_k(dataset, f1_scores, y_min, y_max, y_interval):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset, fontsize=12)
    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel(r'Recall (\%)', fontsize=11)
    ax.set_yticks(range(math.floor(y_min), 101, y_interval))
    ax.set_ylim(y_min, y_max)
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.plot(k_vals, f1_scores['H2-ALSH'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, f1_scores['SA-ALSH'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)

    figs.tight_layout(rect=[0.135, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_kmips_f1.pdf' % dataset, format='pdf')


if __name__ == "__main__":
    t, min_t = read_csv_f1("results/kmips/Amazon-Auto.tsv")
    print(datasets[0], min_t)
    draw_f1_k(datasets[0], t, 95, 100.3, 1)

    t, min_t = read_csv_f1("results/kmips/Amazon-CDs.tsv")
    print(datasets[1], min_t)
    draw_f1_k(datasets[1], t, 80, 101, 5)

    t, min_t = read_csv_f1("results/kmips/MovieLens.tsv")
    print(datasets[2], min_t)
    draw_f1_k(datasets[2], t, 95, 100.3, 1)

    t, min_t = read_csv_f1("results/kmips/Music100.tsv")
    print(datasets[3], min_t)
    draw_f1_k(datasets[3], t, 80, 101, 5)

    t, min_t = read_csv_f1("results/kmips/Netflix.tsv")
    print(datasets[4], min_t)
    draw_f1_k(datasets[4], t, 95, 100.3, 1)

    t, min_t = read_csv_f1("results/kmips/Deep.tsv")
    print(datasets[5], min_t)
    draw_f1_k(datasets[5], t, 85, 101, 3)

    t, min_t = read_csv_f1("results/kmips/Gist.tsv")
    print(datasets[6], min_t)
    draw_f1_k(datasets[6], t, 95, 100.3, 1)

    t, min_t = read_csv_f1("results/kmips/GloVe.tsv")
    print(datasets[7], min_t)
    draw_f1_k(datasets[7], t, 50, 103, 10)

    t, min_t = read_csv_f1("results/kmips/ImageNet.tsv")
    print(datasets[8], min_t)
    draw_f1_k(datasets[8], t, 70, 102, 5)

    t, min_t = read_csv_f1("results/kmips/Msong.tsv")
    print(datasets[9], min_t)
    draw_f1_k(datasets[9], t, 95, 100.3, 1)

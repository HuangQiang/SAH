import csv
import math

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-Books', 'Amazon-CDs', r'Amazon-Movie\&TV', 'Amazon-Tools', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['Simpfer', 'H2-Simpfer', 'SAH', 'H2-ALSH']
markers = ['s', 'o', '^', 'x']
colors = ['forestgreen', 'blue', 'red', 'black']

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
    for alg in ['Simpfer', 'H2', 'H2P', 'SAH']:
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


def draw_f1_k(dataset, f1_scores, y_min, y_max, y_interval):
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

    if len(f1_scores['H2']) > 0:
        ax.plot(k_vals, f1_scores['H2'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    else:
        ax.plot([1, 5], [92.337, 90.386], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    ax.plot(k_vals, f1_scores['Simpfer'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, f1_scores['H2P'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)
    ax.plot(k_vals, f1_scores['SAH'], color=colors[2], linewidth=1.5, fillstyle='none', marker=markers[2], ms=7.5, mew=1.5)

    figs.tight_layout(rect=[0.135, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_k_f1.pdf' % dataset, format='pdf')
    # plt.savefig('../figures/Amazon-Movie_k_f1.pdf', format='pdf')


if __name__ == "__main__":
    t, min_t = read_csv_f1("results/Automotive/varying_k.tsv")
    print(datasets[0], min_t)
    # draw_f1_k(datasets[0], t, 92, 100.5, 2)

    t, min_t = read_csv_f1("results/Books/varying_k.tsv")
    print(datasets[1], min_t)
    # draw_f1_k(datasets[1], t, 80, 101, 5)

    t, min_t = read_csv_f1("results/CDs/varying_k.tsv")
    print(datasets[2], min_t)
    draw_f1_k(datasets[2], t, 60, 102, 10)

    t, min_t = read_csv_f1("results/Movies_and_TV/varying_k.tsv")
    print(datasets[3], min_t)
    # draw_f1_k(datasets[3], t, 80, 101, 5)

    t, min_t = read_csv_f1("results/Tools/varying_k.tsv")
    print(datasets[4], min_t)
    # draw_f1_k(datasets[4], t, 75, 102, 5)

    t, min_t = read_csv_f1("results/MovieLens/varying_k.tsv")
    print(datasets[5], min_t)
    # draw_f1_k(datasets[5], t, 95, 100.3, 1)

    t, min_t = read_csv_f1("results/Music100/varying_k.tsv")
    print(datasets[6], min_t)
    # draw_f1_k(datasets[6], t, 85, 101, 3)

    t, min_t = read_csv_f1("results/Netflix/varying_k.tsv")
    print(datasets[7], min_t)
    # draw_f1_k(datasets[7], t, 95, 100.3, 1)

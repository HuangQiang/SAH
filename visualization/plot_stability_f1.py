import csv
import math

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

# datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
datasets = ['Amazon-CDs', 'MovieLens', 'Netflix']
ratios = ['19', '37', '55', '73', '91']
percents = {'19': r'10\%', '37': r'30\%', '55': r'50\%', '73': r'70\%', '91': r'90\%'}
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
    for alg in algorithms:
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


def draw_f1_k(dataset, ratio, f1_scores, y_min, y_max, y_interval):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset + "(" + percents[ratio] + ")", fontsize=12)

    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel(r'F1-Score (\%)', fontsize=11)
    ax.set_yticks(range(math.floor(y_min), 101, y_interval))
    ax.set_ylim(y_min, y_max)
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.plot(k_vals, f1_scores['H2-ALSH'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5,
            mew=1.5)
    ax.plot(k_vals, f1_scores['Simpfer'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5,
            mew=1.5)
    ax.plot(k_vals, f1_scores['H2-Simpfer'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5,
            mew=1.5)
    ax.plot(k_vals, f1_scores['SAH'], color=colors[2], linewidth=1.5, fillstyle='none', marker=markers[2], ms=7.5,
            mew=1.5)

    figs.tight_layout(rect=[0.135, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_%s_f1.pdf' % (dataset, ratio), format='pdf')


if __name__ == "__main__":
    for ratio in ratios:
        t, min_t = read_csv_f1("results/stability/Amazon-CDs_" + ratio + ".tsv")
        print(datasets[0], min_t)
        draw_f1_k(datasets[0], ratio, t, 75, 101, 5)

        t, min_t = read_csv_f1("results/stability/MovieLens_" + ratio + ".tsv")
        print(datasets[1], min_t)
        draw_f1_k(datasets[1], ratio, t, 95, 100.3, 1)

        t, min_t = read_csv_f1("results/stability/Netflix_" + ratio + ".tsv")
        print(datasets[2], min_t)
        draw_f1_k(datasets[2], ratio, t, 90, 100.5, 2)

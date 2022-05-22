import csv
import numpy as np

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['SF+H2', 'SAH']
colors = ['blue', 'red']

b_vals = ['0.1', '0.3', '0.5', '0.7', '0.9']

plt.rcParams.update({
    "font.size": 10,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})


def read_csv_f1(fname):
    min_f1 = 100
    f1_scores = {}
    for alg in ['H2P', 'SAH']:
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


def draw_f1_b(dataset, f1_scores, k, min_f1):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    xs = np.arange(5)
    width = 0.32
    space = 0.01

    ax.set_title(dataset, fontsize=11)
    ax.set_xlabel(r'$b$', fontsize=12)
    ax.set_xlim(-0.5, 4.5)
    ax.set_xticks(xs)
    ax.set_xticklabels(b_vals)

    ax.set_ylabel(r'F1-Score (\%)', fontsize=11)
    if min_t > 80:
        y_min = 80
        interval = 5
    else:
        y_min = min_t // 10 * 10
        if (100 - y_min) / 5 > 6:
            interval = 10
        else:
            interval = 5
    ax.set_yticks(range(int(y_min), 101, interval))
    ax.set_ylim(y_min, 100)
    ax.yaxis.set_minor_locator(AutoMinorLocator())
    
    ax.bar(xs - width / 2 - space, f1_scores['H2P'], width, color=colors[0])
    ax.bar(xs + width / 2 + space, f1_scores['SAH'], width, color=colors[1])
    
    figs.tight_layout(rect=[0.135, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)
    
    plt.savefig('../figures/%s_b_k%s_f1.pdf' % (dataset, k), format='pdf')


if __name__ == "__main__":
    k = 10
    t, min_t = read_csv_f1("results/Automotive/varying_b_k%s.tsv" % k)
    print(min_t)
    draw_f1_b(datasets[0], t, k, min_t)
    
    t, min_t = read_csv_f1("results/CDs/varying_b_k%s.tsv" % k)
    print(min_t)
    draw_f1_b(datasets[2], t, k, min_t)
    
    t, min_t = read_csv_f1("results/MovieLens/varying_b_k%s.tsv" % k)
    print(min_t)
    # draw_f1_b(datasets[5], t, k, min_t)

    t, min_t = read_csv_f1("results/Music100/varying_b_k%s.tsv" % k)
    print(min_t)
    # draw_f1_b(datasets[6], t, k, min_t)

    t, min_t = read_csv_f1("results/Netflix/varying_b_k%s.tsv" % k)
    print(min_t)
    # draw_f1_b(datasets[7], t, k, min_t)

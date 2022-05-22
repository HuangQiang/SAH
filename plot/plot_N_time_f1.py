import csv
import math

import numpy as np

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
measures = ['Time (ms)', r'F1-Score (\%)']
colors = ['blue', 'red']

N_vals = ['20', '50', '100', '200']

plt.rcParams.update({
    "font.size": 9,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})


def read_csv_time_f1(fname):
    max_time = 0
    min_f1 = 100
    q_time = {}
    f1_scores = {}
    for alg in ['SAH']:
        q_time[alg] = []
        f1_scores[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            time = float(row[2])
            f1 = float(row[3])
            q_time[alg].append(time)
            f1_scores[alg].append(f1)
            if time > max_time:
                max_time = time
            if f1 < min_f1:
                min_f1 = f1
    return q_time, f1_scores, max_time, min_f1


def draw_time_f1_N(dataset, q_time, f1_scores, k, y1_min, y1_max, y1_int, y1_scale, y2_min, y2_max, y2_int):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    xs = np.arange(4)
    width = 0.5

    ax.set_title(dataset, fontsize=10)
    ax.set_xlabel(r'$N_0$', fontsize=10)
    ax.set_xlim(-0.3, 3.3)
    ax.set_xticks(xs)
    ax.set_xticklabels(N_vals)

    if y1_scale == 0:
        ax.set_ylabel('Time (ms)', fontsize=10, color=colors[1])
    else:
        ax.set_ylabel(r'Time (ms, $\times 10^%s$)' % y1_scale, fontsize=10, color=colors[1])

    ax.set_yticks(np.arange(int(y1_min), int(y1_max + 1), y1_int))
    ax.set_ylim(y1_min, y1_max)
    ax.tick_params(axis='y', labelcolor=colors[1])
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.bar(xs, [x / math.pow(10, y1_scale) for x in q_time['SAH']], width, color=colors[1])

    ax2 = ax.twinx()
    ax2.set_ylabel(r'F1-Score (\%)', fontsize=10, color=colors[0])
    ax2.set_yticks(np.arange(int(y2_min), int(y2_max + 1), y2_int))
    ax2.set_ylim(y2_min, y2_max)
    ax2.tick_params(axis='y', labelcolor=colors[0])
    ax2.yaxis.set_minor_locator(AutoMinorLocator())

    ax2.plot(xs, f1_scores['SAH'], color=colors[0], linewidth=1.5, marker='.', ms=10, mew=1.5)

    figs.tight_layout(rect=[0.12, 0.06, 0.88, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.savefig('../figures/%s_N_k%s_time.pdf' % (dataset, k), format='pdf')


if __name__ == "__main__":
    k = 10
    t, f, max_t, min_f = read_csv_time_f1("results/Automotive/varying_N_k%s.tsv" % k)
    print(max_t, min_f)
    draw_time_f1_N(datasets[0], t, f, k, 3, 4, 0.2, 2, 90, 100.5, 2)

    t, f, max_t, min_f = read_csv_time_f1("results/CDs/varying_N_k%s.tsv" % k)
    print(max_t, min_f)
    draw_time_f1_N(datasets[1], t, f, k, 0.5, 1, 0.1, 2, 90, 100.5, 2)

    t, f, max_t, min_f = read_csv_time_f1("results/MovieLens/varying_N_k%s.tsv" % k)
    print(max_t, min_f)
    draw_time_f1_N(datasets[2], t, f, k, 0, 0.8, 0.2, 0, 90, 100.5, 2)

    t, f, max_t, min_f = read_csv_time_f1("results/Music100/varying_N_k%s.tsv" % k)
    print(max_t, min_f)
    draw_time_f1_N(datasets[3], t, f, k, 2, 3, 0.2, 3, 90, 100.5, 2)

    t, f, max_t, min_f = read_csv_time_f1("results/Netflix/varying_N_k%s.tsv" % k)
    print(max_t, min_f)
    draw_time_f1_N(datasets[4], t, f, k, 0, 6, 1, 0, 90, 100.5, 2)

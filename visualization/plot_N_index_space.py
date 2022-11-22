import csv
import math

import numpy as np

import matplotlib.pyplot as plt
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
measures = ['Index Time (s)', r'Memory (MB)']
colors = ['blue', 'red']

N_vals = ['20', '50', '100', '200']

plt.rcParams.update({
    "font.size": 9,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})


def read_csv_index_space(fname):
    max_index = 0
    max_space = 0
    index_time = {}
    index_space = {}
    for alg in ['SAH']:
        index_time[alg] = []
        index_space[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            time = float(row[4])
            space = float(row[5])
            index_time[alg].append(time)
            index_space[alg].append(space)
            if time > max_index:
                max_index = time
            if space > max_space:
                max_space = space
    return index_time, index_space, max_index, max_space


def draw_index_space_N(dataset, index_time, index_space, k, y1_min, y1_max, y1_int, y1_scale, y2_min, y2_max, y2_int, y2_scale):
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
        ax.set_ylabel('Index Time (s)', fontsize=10, color=colors[1])
    else:
        ax.set_ylabel(r'Index Time (s, $\times 10^%s$)' % y1_scale, fontsize=10, color=colors[1])

    ax.set_yticks(np.arange(int(y1_min), int(y1_max + 1), y1_int))
    ax.set_ylim(y1_min, y1_max)
    ax.tick_params(axis='y', labelcolor=colors[1])
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.bar(xs, [x / math.pow(10, y1_scale) for x in index_time['SAH']], width, color=colors[1])

    ax2 = ax.twinx()
    if y2_scale == 0:
        ax2.set_ylabel('Memory (MB)', fontsize=10, color=colors[0])
    else:
        ax2.set_ylabel(r'Memory (MB, $\times 10^%s$)' % y2_scale, fontsize=10, color=colors[0])
    ax2.set_yticks(np.arange(int(y2_min), int(y2_max + 1), y2_int))
    ax2.set_ylim(y2_min, y2_max)
    ax2.tick_params(axis='y', labelcolor=colors[0])
    ax2.yaxis.set_minor_locator(AutoMinorLocator())
    if y2_scale == 0:
        ax2.plot(xs, index_space['SAH'], color=colors[0], linewidth=1.5, marker='.', ms=10, mew=1.5)
    else:
        ax2.plot(xs, [x / math.pow(10, y2_scale) for x in index_space['SAH']], color=colors[0], linewidth=1.5, marker='.', ms=10, mew=1.5)

    figs.tight_layout(rect=[0.12, 0.06, 0.88, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.savefig('../figures/%s_N_k%s_index.pdf' % (dataset, k), format='pdf')


if __name__ == "__main__":
    k = 10
    t, s, max_t, max_s = read_csv_index_space("results/Automotive/varying_N_k%s.tsv" % k)
    print(max_t, max_s)
    draw_index_space_N(datasets[0], t, s, k, 3, 4.5, 0.3, 2, 0, 1.6, 0.4, 3)

    t, s, max_t, max_s = read_csv_index_space("results/CDs/varying_N_k%s.tsv" % k)
    print(max_t, max_s)
    draw_index_space_N(datasets[1], t, s, k, 0, 16, 4, 0, 0, 30, 5, 0)

    t, s, max_t, max_s = read_csv_index_space("results/MovieLens/varying_N_k%s.tsv" % k)
    print(max_t, max_s)
    draw_index_space_N(datasets[2], t, s, k, 0, 4, 1, 0, 0, 25, 5, 0)

    t, s, max_t, max_s = read_csv_index_space("results/Music100/varying_N_k%s.tsv" % k)
    print(max_t, max_s)
    draw_index_space_N(datasets[3], t, s, k, 30, 50, 5, 0, 0, 4, 1, 2)

    t, s, max_t, max_s = read_csv_index_space("results/Netflix/varying_N_k%s.tsv" % k)
    print(max_t, max_s)
    draw_index_space_N(datasets[4], t, s, k, 0, 30, 5, 0, 0, 1.6, 0.4, 2)

import csv
import sys

import numpy as np

import matplotlib.pyplot as plt
from matplotlib import ticker
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-Books', 'Amazon-CDs', r'Amazon-Movie\&TV', 'Amazon-Tools', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['SF+H2', 'SAH']
colors = ['blue', 'red']

b_vals = ['0.1', '0.3', '0.5', '0.7', '0.9']

plt.rcParams.update({
    "font.size": 10,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})


def read_csv_time(fname):
    min_time = sys.maxsize
    max_time = 0
    q_time = {}
    for alg in ['H2P', 'SAH']:
        q_time[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            time = float(row[2])
            q_time[alg].append(time)
            if time > max_time:
                max_time = time
            if time < min_time:
                min_time = time
    return q_time, min_time, max_time


def draw_time_b(dataset, f1_scores, k, y_min, y_max, y_int):
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

    ax.set_ylabel('Time (ms)', fontsize=11)
    ax.set_yticks(range(int(y_min), int(y_max + 1), y_int))
    ax.set_ylim(y_min, y_max)
    if y_max > 99:
        formatter = ticker.ScalarFormatter(useMathText=True)
        formatter.set_scientific(True)
        formatter.set_powerlimits((-1, 1))
        ax.yaxis.set_major_formatter(formatter)
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.bar(xs - width / 2 - space, f1_scores['H2P'], width, color=colors[0])
    ax.bar(xs + width / 2 + space, f1_scores['SAH'], width, color=colors[1])

    figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.savefig('../figures/%s_b_k%s_time.pdf' % (dataset, k), format='pdf')
    # plt.savefig('../figures/Amazon-Movie_b_k%s_time.pdf' % k, format='pdf')


if __name__ == "__main__":
    k = 10
    t, min_t, max_t = read_csv_time("results/Automotive/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[0], t, k, 0, 1000, 200)

    t, min_t, max_t = read_csv_time("results/Books/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[1], t, k, 0, 3000, 500)

    t, min_t, max_t = read_csv_time("results/CDs/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    draw_time_b(datasets[2], t, k, 0, 420, 100)

    t, min_t, max_t = read_csv_time("results/Movies_and_TV/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[3], t, k, 0, 300, 50)

    t, min_t, max_t = read_csv_time("results/Tools/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[4], t, k, 0, 700, 200)

    t, min_t, max_t = read_csv_time("results/MovieLens/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[5], t, k, 0, 3.2, 1)

    t, min_t, max_t = read_csv_time("results/Music100/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[6], t, k, 0, 22000, 5000)

    t, min_t, max_t = read_csv_time("results/Netflix/varying_b_k%s.tsv" % k)
    print(min_t, max_t)
    # draw_time_b(datasets[7], t, k, 0, 27, 5)

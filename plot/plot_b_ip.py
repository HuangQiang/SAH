import csv
import sys

import numpy as np

import matplotlib.pyplot as plt
from matplotlib import ticker
from matplotlib.ticker import AutoMinorLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['SF+H2', 'SAH']
colors = ['blue', 'red']

b_vals = ['0.1', '0.3', '0.5', '0.7', '0.9']

plt.rcParams.update({
    "font.size": 12,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{sfmath}",
    "font.sans-serif": ["Helvetica"]})


def read_csv_ip(fname):
    ip_comps = {}
    min_ip = sys.maxsize
    max_ip = 0
    for alg in ['H2P', 'SAH']:
        ip_comps[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            ips = int(row[3])
            ip_comps[alg].append(ips)
            if ips > max_ip:
                max_ip = ips
            if ips < min_ip:
                min_ip = ips
    return ip_comps, min_ip, max_ip


def draw_ip_b(dataset, f1_scores, k, y_min, y_max, y_int):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    xs = np.arange(5)
    width = 0.32
    space = 0.01

    ax.set_xlabel(r'$b$', fontsize=14)
    ax.set_xlim(-0.5, 4.5)
    ax.set_xticks(xs)
    ax.set_xticklabels(b_vals)

    ax.set_ylabel(r'\#IP Computations', fontsize=14)
    ax.set_yticks(range(y_min, y_max, y_int))
    ax.set_ylim(y_min, y_max)
    formatter = ticker.ScalarFormatter(useMathText=True)
    formatter.set_scientific(True)
    formatter.set_powerlimits((-1, 1))
    ax.yaxis.set_major_formatter(formatter)
    ax.yaxis.set_minor_locator(AutoMinorLocator())

    ax.bar(xs - width / 2 - space, f1_scores['H2P'], width, color=colors[0])
    ax.bar(xs + width / 2 + space, f1_scores['SAH'], width, color=colors[1])

    figs.tight_layout(rect=[0.135, 0.06, 1, 0.99])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.savefig('../figures/%s_b_k%s_ip.pdf' % (dataset, k), format='pdf')


if __name__ == "__main__":
    k = 10
    i, min_i, max_i = read_csv_ip("../results/Automotive/varying_b_k%s.tsv" % k)
    print(min_i, max_i)
    # draw_ip_b(datasets[0], i, k, 300000, 800001, 100000)
    # draw_ip_b(datasets[0], i, k, 600000, 1000001, 100000)
    draw_ip_b(datasets[0], i, k, 1000000, 1500001, 100000)

    i, min_i, max_i = read_csv_ip("../results/Books/varying_b_k%s.tsv" % k)
    print(min_i, max_i)
    # draw_ip_b(datasets[1], i, k, 0, 300001, 100000)
    # draw_ip_b(datasets[1], i, k, 0, 600001, 100000)
    draw_ip_b(datasets[1], i, k, 0, 800001, 200000)

    i, min_i, max_i = read_csv_ip("../results/Music100/varying_b_k%s.tsv" % k)
    print(min_i, max_i)
    # draw_ip_b(datasets[2], i, k, 800000, 1500001, 100000)
    # draw_ip_b(datasets[2], i, k, 2000000, 3200001, 500000)
    draw_ip_b(datasets[2], i, k, 3000000, 6000001, 500000)

    i, min_i, max_i = read_csv_ip("../results/Netflix/varying_b_k%s.tsv" % k)
    print(min_i, max_i)
    # draw_ip_b(datasets[3], i, k, 0, 30001, 10000)
    # draw_ip_b(datasets[3], i, k, 0, 50001, 10000)
    draw_ip_b(datasets[3], i, k, 0, 80001, 20000)

    i, min_i, max_i = read_csv_ip("../results/Tools/varying_b_k%s.tsv" % k)
    print(min_i, max_i)
    # draw_ip_b(datasets[4], i, k, 300000, 900001, 100000)
    # draw_ip_b(datasets[4], i, k, 700000, 1200001, 100000)
    draw_ip_b(datasets[4], i, k, 1000000, 2000001, 200000)

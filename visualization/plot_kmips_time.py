import csv
import sys

import matplotlib.pyplot as plt

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


def read_csv_time(fname):
    min_time = sys.maxsize
    max_time = 0
    q_time = {}
    for alg in ['H2-ALSH', 'SA-ALSH']:
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


def draw_time_k(dataset, q_time, y_min, y_max):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset, fontsize=12)
    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel('Time (ms)', fontsize=11)
    ax.set_ylim(y_min, y_max)
    ax.set_yscale('log')

    ax.plot(k_vals, q_time['H2-ALSH'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['SA-ALSH'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)

    if y_min < 1:
        figs.tight_layout(rect=[0.15, 0.06, 1, 0.98])
    else:
        figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_kmips_time.pdf' % dataset, format='pdf')


if __name__ == "__main__":
    t, _, _ = read_csv_time("results/kmips/Amazon-Auto.tsv")
    draw_time_k(datasets[0], t, 0.01, 0.4)

    t, _, _ = read_csv_time("results/kmips/Amazon-CDs.tsv")
    draw_time_k(datasets[1], t, 0.1, 2)

    t, _, _ = read_csv_time("results/kmips/MovieLens.tsv")
    draw_time_k(datasets[2], t, 0.01, 0.4)

    t, _, _ = read_csv_time("results/kmips/Music100.tsv")
    draw_time_k(datasets[3], t, 1, 100)

    t, _, _ = read_csv_time("results/kmips/Netflix.tsv")
    draw_time_k(datasets[4], t, 0.01, 0.4)

    t, _, _ = read_csv_time("results/kmips/Deep.tsv")
    draw_time_k(datasets[5], t, 10, 200)

    t, _, _ = read_csv_time("results/kmips/Gist.tsv")
    draw_time_k(datasets[6], t, 0.1, 2)

    t, _, _ = read_csv_time("results/kmips/GloVe.tsv")
    draw_time_k(datasets[7], t, 10, 200)

    t, _, _ = read_csv_time("results/kmips/ImageNet.tsv")
    draw_time_k(datasets[8], t, 5, 100)

    t, _, _ = read_csv_time("results/kmips/Msong.tsv")
    draw_time_k(datasets[9], t, 0.1, 2)


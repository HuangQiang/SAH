import csv
import sys

import matplotlib.pyplot as plt
from matplotlib.ticker import MultipleLocator

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['Simpfer', 'H2-Simpfer', 'SAH', 'H2-ALSH']
markers = ['s', 'o', '^', 'x']
colors = ['forestgreen', 'blue', 'red', 'black']

plt.rcParams.update({
    "font.size": 10,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})


def read_csv(fname):
    q_f1 = {}
    q_time = {}
    for alg in ['Simpfer', 'H2-ALSH', 'H2-Simpfer', 'SAH']:
        q_f1[alg] = []
        q_time[alg] = []
    with open(fname, newline='') as f:
        reader = csv.reader(f, delimiter='\t')
        i = 0
        for row in reader:
            i += 1
            if i == 1:
                continue
            alg = row[0]
            f1 = float(row[2])
            time = float(row[1])
            q_f1[alg].append(f1)
            q_time[alg].append(time)
    return q_f1, q_time


def draw_time_accuracy(dataset, q_f1, q_time, y_min, y_max):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset, fontsize=12)
    ax.set_xlabel(r'F1-Score (\%)', fontsize=11)
    ax.set_xlim(-5, 105)
    ax.set_xticks(range(0, 110, 20))
    ax.xaxis.set_minor_locator(MultipleLocator(10))

    ax.set_ylabel('Time (ms)', fontsize=11)
    ax.set_ylim(y_min, y_max)
    ax.set_yscale('log')

    ax.plot(q_f1['H2-ALSH'], q_time['H2-ALSH'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    ax.plot(q_f1['Simpfer'], q_time['Simpfer'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(q_f1['H2-Simpfer'], q_time['H2-Simpfer'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)
    ax.plot(q_f1['SAH'], q_time['SAH'], color=colors[2], linewidth=1.5, fillstyle='none', marker=markers[2], ms=7.5, mew=1.5)

    if y_min < 1:
        figs.tight_layout(rect=[0.15, 0.06, 1, 0.98])
    else:
        figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_time_f1.pdf' % dataset, format='pdf')


if __name__ == "__main__":
    acc, t = read_csv("results/Automotive/time-accuracy.tsv")
    draw_time_accuracy(datasets[0], acc, t, 8, 3e4)

    acc, t = read_csv("results/CDs/time-accuracy.tsv")
    draw_time_accuracy(datasets[1], acc, t, 4, 2000)

    acc, t = read_csv("results/MovieLens/time-accuracy.tsv")
    draw_time_accuracy(datasets[2], acc, t, 0.1, 200)

    acc, t = read_csv("results/Music100/time-accuracy.tsv")
    draw_time_accuracy(datasets[3], acc, t, 50, 2e5)

    acc, t = read_csv("results/Netflix/time-accuracy.tsv")
    draw_time_accuracy(datasets[4], acc, t, 0.2, 1e3)

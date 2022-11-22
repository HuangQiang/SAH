import csv
import sys

import matplotlib.pyplot as plt

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


def read_csv_time(fname):
    min_time = sys.maxsize
    max_time = 0
    q_time = {}
    for alg in algorithms:
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


def draw_time_k(dataset, ratio, q_time, y_min, y_max):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset + "(" + percents[ratio] + ")", fontsize=12)

    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel('Time (ms)', fontsize=11)
    ax.set_ylim(y_min, y_max)
    ax.set_yscale('log')

    ax.plot(k_vals, q_time['H2-ALSH'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['Simpfer'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['H2-Simpfer'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['SAH'], color=colors[2], linewidth=1.5, fillstyle='none', marker=markers[2], ms=7.5, mew=1.5)

    if y_min < 1:
        figs.tight_layout(rect=[0.15, 0.06, 1, 0.98])
    else:
        figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_%s_time.pdf' % (dataset, ratio), format='pdf')


if __name__ == "__main__":
    for ratio in ratios:
        t, _, _ = read_csv_time("results/stability/Amazon-CDs_" + ratio + ".tsv")
        draw_time_k(datasets[0], ratio, t, 10, 2e4)

        t, _, _ = read_csv_time("results/stability/MovieLens_" + ratio + ".tsv")
        draw_time_k(datasets[1], ratio, t, 1, 4e3)

        t, _, _ = read_csv_time("results/stability/Netflix_" + ratio + ".tsv")
        draw_time_k(datasets[2], ratio, t, 10, 3e4)

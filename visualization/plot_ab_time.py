import csv
import sys

import matplotlib.pyplot as plt

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
algorithms = ['H2-Simpfer', 'H2-Cone', 'SA-Simpfer', 'SAH']
markers = ['o', '^', '.', 'D']
colors = ['blue', 'red', 'purple', 'darkorange']

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
    for alg in ['SAP', 'H2P', 'SAH', 'H2C']:
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


def draw_time_a(dataset, q_time, y_min, y_max):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset, fontsize=12)
    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel('Time (ms)', fontsize=11)
    ax.set_ylim(y_min, y_max)
    ax.set_yscale('log')

    ax.plot(k_vals, q_time['H2P'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['SAP'], color=colors[2], linewidth=1.5, marker=markers[2], ms=9, mew=1.5)
    ax.plot(k_vals, q_time['H2C'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['SAH'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)

    if y_min < 1:
        figs.tight_layout(rect=[0.14, 0.06, 1, 0.98])
    else:
        figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_ablation_time.pdf' % dataset, format='pdf')


if __name__ == "__main__":
    t, min_t, max_t = read_csv_time("results/Automotive/ablation.tsv")
    print(datasets[0], min_t, max_t)
    draw_time_a(datasets[0], t, 10, 8000)

    t, min_t, max_t = read_csv_time("results/CDs/ablation.tsv")
    print(datasets[1], min_t, max_t)
    draw_time_a(datasets[1], t, 5, 2000)

    t, min_t, max_t = read_csv_time("results/MovieLens/ablation.tsv")
    print(datasets[2], min_t, max_t)
    draw_time_a(datasets[2], t, 0.08, 30)

    t, min_t, max_t = read_csv_time("results/Music100/ablation.tsv")
    print(datasets[3], min_t, max_t)
    draw_time_a(datasets[3], t, 300, 2e5)

    t, min_t, max_t = read_csv_time("results/Netflix/ablation.tsv")
    print(datasets[4], min_t, max_t)
    draw_time_a(datasets[4], t, 0.8, 125)

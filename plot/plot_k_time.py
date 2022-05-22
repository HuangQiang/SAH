import csv
import sys

import matplotlib.pyplot as plt

datasets = ['Amazon-Auto', 'Amazon-CDs', 'MovieLens', 'Music100', 'Netflix']
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
    for alg in ['Simpfer', 'H2', 'H2P', 'SAH']:
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


def calc_speedups(t):
    r1_min = sys.maxsize
    r1_max = 0
    r2_min = sys.maxsize
    r2_max = 0
    for i in range(len(t['SAH'])):
        if t['Simpfer'][i] / t['SAH'][i] > r1_max:
            r1_max = t['Simpfer'][i] / t['SAH'][i]
        if t['Simpfer'][i] / t['SAH'][i] < r1_min:
            r1_min = t['Simpfer'][i] / t['SAH'][i]
        if t['H2P'][i] / t['SAH'][i] > r2_max:
            r2_max = t['H2P'][i] / t['SAH'][i]
        if t['H2P'][i] / t['SAH'][i] < r2_min:
            r2_min = t['H2P'][i] / t['SAH'][i]
    return r1_min, r1_max, r2_min, r2_max


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

    if len(q_time['H2']) > 0:
        ax.plot(k_vals, q_time['H2'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    else:
        ax.plot([1, 5], [592242.864, 697754.617], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['Simpfer'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['H2P'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)
    ax.plot(k_vals, q_time['SAH'], color=colors[2], linewidth=1.5, fillstyle='none', marker=markers[2], ms=7.5, mew=1.5)

    if y_min < 1:
        figs.tight_layout(rect=[0.14, 0.06, 1, 0.98])
    else:
        figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_k_time.pdf' % dataset, format='pdf')
    # plt.savefig('../figures/Amazon-Movie_k_time.pdf', format='pdf')


if __name__ == "__main__":
    t, min_t, max_t = read_csv_time("results/Automotive/varying_k.tsv")
    r1_min, r1_max, r2_min, r2_max = calc_speedups(t)
    print(datasets[0], 'Simpfer', r1_min, r1_max)
    print(datasets[0], 'H2P', r2_min, r2_max)
    print(datasets[0], min_t, max_t)
    draw_time_k(datasets[0], t, 1e1, 1.5e5)
    
    t, min_t, max_t = read_csv_time("results/CDs/varying_k.tsv")
    r1_min, r1_max, r2_min, r2_max = calc_speedups(t)
    print(datasets[2], 'Simpfer', r1_min, r1_max)
    print(datasets[2], 'H2P', r2_min, r2_max)
    print(datasets[2], min_t, max_t)
    draw_time_k(datasets[2], t, 1, 1e4)
    
    t, min_t, max_t = read_csv_time("results/MovieLens/varying_k.tsv")
    r1_min, r1_max, r2_min, r2_max = calc_speedups(t)
    print(datasets[5], 'Simpfer', r1_min, r1_max)
    print(datasets[5], 'H2P', r2_min, r2_max)
    print(datasets[5], min_t, max_t)
    draw_time_k(datasets[5], t, 0.08, 1e3)
    
    t, min_t, max_t = read_csv_time("results/Music100/varying_k.tsv")
    r1_min, r1_max, r2_min, r2_max = calc_speedups(t)
    print(datasets[6], 'Simpfer', r1_min, r1_max)
    print(datasets[6], 'H2P', r2_min, r2_max)
    print(datasets[6], min_t, max_t)
    draw_time_k(datasets[6], t, 1e2, 1e6)

    t, min_t, max_t = read_csv_time("results/Netflix/varying_k.tsv")
    r1_min, r1_max, r2_min, r2_max = calc_speedups(t)
    print(datasets[7], 'Simpfer', r1_min, r1_max)
    print(datasets[7], 'H2P', r2_min, r2_max)
    print(datasets[7], min_t, max_t)
    draw_time_k(datasets[7], t, 0.8, 1e4)

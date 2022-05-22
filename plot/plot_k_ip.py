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


def read_csv_ip(fname):
    ip_comps = {}
    min_ip = sys.maxsize
    max_ip = 0
    for alg in ['Simpfer', 'H2', 'H2P', 'SAH']:
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


def draw_ip_k(dataset, ip_comps, y_min, y_max):
    figs, ax = plt.subplots()

    ax.tick_params(axis='y', which='both', direction='out')

    ax.set_title(dataset, fontsize=12)
    ax.set_xlabel(r'$k$', fontsize=12)
    ax.set_xlim(-2, 53)
    ax.set_xticks([1, 10, 20, 30, 40, 50])

    ax.set_ylabel(r'\#IP Computations', fontsize=11)
    ax.set_ylim(y_min, y_max)
    ax.set_yscale('log')

    if len(ip_comps['H2']) > 0:
        ax.plot(k_vals, ip_comps['H2'], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    else:
        ax.plot([1, 5], [55614699, 66868930], color=colors[3], linewidth=1.5, fillstyle='none', marker=markers[3], ms=7.5, mew=1.5)
    ax.plot(k_vals, ip_comps['Simpfer'], color=colors[0], linewidth=1.5, fillstyle='none', marker=markers[0], ms=7.5, mew=1.5)
    ax.plot(k_vals, ip_comps['H2P'], color=colors[1], linewidth=1.5, fillstyle='none', marker=markers[1], ms=7.5, mew=1.5)
    ax.plot(k_vals, ip_comps['SAH'], color=colors[2], linewidth=1.5, fillstyle='none', marker=markers[2], ms=7.5, mew=1.5)

    figs.tight_layout(rect=[0.13, 0.06, 1, 0.98])
    figs.set_figheight(2.4)
    figs.set_figwidth(2.4)

    plt.grid(axis='both', color='gray', lw=0.3, ls=':', which='both')

    plt.savefig('../figures/%s_k_ip.pdf' % dataset, format='pdf')
    # plt.savefig('../figures/Amazon-Movie_k_ip.pdf', format='pdf')


if __name__ == "__main__":
    t, min_t, max_t = read_csv_ip("results/Automotive/varying_k.tsv")
    print(datasets[0], min_t, max_t)
    draw_ip_k(datasets[0], t, 2e4, 4e8)

    t, min_t, max_t = read_csv_ip("results/CDs/varying_k.tsv")
    print(datasets[2], min_t, max_t)
    draw_ip_k(datasets[2], t, 8e3, 1e7)

    t, min_t, max_t = read_csv_ip("results/MovieLens/varying_k.tsv")
    print(datasets[5], min_t, max_t)
    draw_ip_k(datasets[5], t, 1e2, 6e6)

    t, min_t, max_t = read_csv_ip("results/Music100/varying_k.tsv")
    print(datasets[6], min_t, max_t)
    draw_ip_k(datasets[6], t, 7e5, 4e8)

    t, min_t, max_t = read_csv_ip("results/Netflix/varying_k.tsv")
    print(datasets[7], min_t, max_t)
    draw_ip_k(datasets[7], t, 1e3, 4e7)

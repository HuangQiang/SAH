import matplotlib.pyplot as plt
from matplotlib.lines import Line2D

plt.rcParams.update({
    "font.size": 14,
    "text.usetex": True,
    "font.family": "sans-serif",
    "text.latex.preamble": r"\usepackage{arev}",
    "font.sans-serif": ["Helvetica"]})

algorithms = ['H2-ALSH', 'H2-Simpfer', 'SAH', 'Simpfer', 'SA-Simpfer']
markers = ['x', 'o', '^', 's', '.']
colors = ['black', 'blue', 'red', 'forestgreen', 'purple']


def draw_legend_4():
    figs, ax = plt.subplots()
    stats = [0]
    x_labels = algorithms

    lines = []
    for i in range(4):
        line = Line2D(range(1), stats, color=colors[i], linewidth=1.5, fillstyle='none', marker=markers[i], ms=10, mew=1.5)
        lines.append(line)

    leg = plt.figlegend(lines, x_labels, loc='upper center', ncol=4, columnspacing=1.0, handlelength=2.5)
    leg.get_frame().set_linewidth(0.0)

    figs.tight_layout()
    figs.set_figheight(5)
    figs.set_figwidth(12.5)

    plt.subplots_adjust(top=0.7)
    plt.savefig('../figures/legend-4.pdf', format='pdf')


def draw_legend_bar():
    figs, ax = plt.subplots()
    stats = [0]
    width = 0.32
    x_labels = [algorithms[1], algorithms[2]]

    rects = []
    for i in [1, 2]:
        rect = ax.bar(0, stats, width, color=colors[i])
        rects.append(rect)

    leg = plt.figlegend(rects, x_labels, loc='upper center', ncol=2, columnspacing=1.0, handlelength=2.5)
    leg.get_frame().set_linewidth(0.0)

    figs.tight_layout()
    figs.set_figheight(5)
    figs.set_figwidth(12.5)

    plt.subplots_adjust(top=0.7)
    plt.savefig('../figures/legend-bar.pdf', format='pdf')


def draw_legend_3():
    figs, ax = plt.subplots()
    stats = [0]
    x_labels = algorithms[1:]

    lines = []
    for i in range(1, 4):
        line = Line2D(range(1), stats, color=colors[i], linewidth=1.5, fillstyle='none', marker=markers[i], ms=10, mew=1.5)
        lines.append(line)

    leg = plt.figlegend(lines, x_labels, loc='upper center', ncol=3, columnspacing=1.0, handlelength=2.5)
    leg.get_frame().set_linewidth(0.0)

    figs.tight_layout()
    figs.set_figheight(5)
    figs.set_figwidth(12.5)

    plt.subplots_adjust(top=0.7)
    plt.savefig('../figures/legend-3.pdf', format='pdf')


def draw_legend_a():
    figs, ax = plt.subplots()
    stats = [0]
    x_labels = [algorithms[1], algorithms[4], algorithms[2]]

    lines = []
    for i in [1, 2]:
        line = Line2D(range(1), stats, color=colors[i], linewidth=1.5, fillstyle='none', marker=markers[i], ms=10, mew=1.5)
        lines.append(line)
    line = Line2D(range(1), stats, color=colors[4], linewidth=1.5, marker=markers[4], ms=12.5, mew=1.5)
    lines.append(line)

    leg = plt.figlegend([lines[0], lines[2], lines[1]], x_labels, loc='upper center', ncol=3, columnspacing=1.0, handlelength=2.5)
    leg.get_frame().set_linewidth(0.0)

    figs.tight_layout()
    figs.set_figheight(5)
    figs.set_figwidth(12.5)

    plt.subplots_adjust(top=0.7)
    plt.savefig('../figures/legend-a.pdf', format='pdf')


if __name__ == "__main__":
    draw_legend_a()
    # draw_legend_4()
    # draw_legend_bar()

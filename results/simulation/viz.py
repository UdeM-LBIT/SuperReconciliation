#!/usr/bin/env python3

import os

import json
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

cur_dir = os.path.dirname(os.path.abspath(__file__))
lengths = range(1, 11)

plt.rc('text', usetex=True)

[_, plots] = plt.subplots(
    nrows=11, ncols=2,
    sharey='col',
    figsize=(16, 50), dpi=80)

i = 0
sub_dir_list = sorted(os.listdir(cur_dir))

for sub_dir in sub_dir_list:
    if sub_dir.startswith('p-'):
        probability = sub_dir.replace('p-', '')
        sub_dir_path = os.path.join(cur_dir, sub_dir)

        plt.figtext(
            0.5, 0.985 - ((0.975 / 11) * i),
            r'Results for $p_{loss} = ' + probability + '$',
            size='xx-large',
            ha='center', va='center')

        with open(os.path.join(sub_dir_path, 'distances.json'), 'r') as in_file:
            distances = json.load(in_file)

            plt.sca(plots[i, 0])
            plt.title('Distance from the reference to the reconciled tree')
            plt.xlabel('Number of gene families in the ancestral synteny')
            plt.ylabel('Edit distance')

            # Box plot of edit distances
            plt.boxplot(distances, positions=lengths)

        with open(os.path.join(sub_dir_path, 'durations.json'), 'r') as in_file:
            durations = json.load(in_file)

            plt.sca(plots[i, 1])
            plt.title('Reconciled tree computation time')
            plt.xlabel('Number of gene families in the ancestral synteny')
            plt.ylabel('Time (s)')

            # Fit an exponential function to the average durations
            avg_durations = list(map(np.average, durations))
            exp = lambda x, a, b : a * np.exp(x * b)
            [params, _] = opt.curve_fit(exp, lengths, avg_durations)

            x = np.linspace(1, 11, 256, endpoint=True)
            y = list(map(lambda x : exp(x, *params), x))

            plt.plot(
                x, y,
                linestyle='dashed', linewidth=1,
                label=r'Best fit exponential'.format(*params))

            # Overlay a box plot of durations
            box = plt.boxplot(durations, positions=lengths, patch_artist=True)

            for patch in box['boxes']:
                patch.set_facecolor('white')

            plt.legend()

        i += 1

plt.tight_layout(pad=8)
plt.savefig(os.path.join(cur_dir, 'viz.pdf'))

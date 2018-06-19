#!/usr/bin/env python3

import os

import json
import numpy as np
import matplotlib.pyplot as plt

cur_dir = os.path.dirname(os.path.abspath(__file__))
lengths = range(1, 11)

plot_index = 1
plot_rows = 11
plot_cols = 2

plt.rc('text', usetex=True)
plt.figure(figsize=(12, 40), dpi=80)

last_distances_plot = None
last_durations_plot = None

sub_dir_list = sorted(os.listdir(cur_dir))

for sub_dir in sub_dir_list:
    if sub_dir.startswith('p-'):
        sub_dir_path = os.path.join(cur_dir, sub_dir)

        with open(os.path.join(sub_dir_path, 'distances.json'), 'r') as in_file:
            distances = json.load(in_file)

        with open(os.path.join(sub_dir_path, 'durations.json'), 'r') as in_file:
            durations = json.load(in_file)

        if last_distances_plot is None:
            last_distances_plot = plt.subplot(plot_rows, plot_cols, plot_index)
        else:
            last_distances_plot = plt.subplot(
                plot_rows, plot_cols, plot_index,
                sharey=last_distances_plot)

        plt.title('Distance from the reference to the reconciled tree\n'
            + r'$loss\_probability = ' + sub_dir.replace('p-', '') + '$')
        plot_index += 1

        plt.xlabel('Number of gene families in the ancestral synteny')
        plt.ylabel('Edit distance')
        plt.boxplot(distances, positions=lengths)

        if last_durations_plot is None:
            last_durations_plot = plt.subplot(plot_rows, plot_cols, plot_index)
        else:
            last_durations_plot = plt.subplot(
                plot_rows, plot_cols, plot_index,
                sharey=last_durations_plot)

        plt.title('Reconciled tree computation time\n'
            + r'$loss\_probability = ' + sub_dir.replace('p-', '') + '$')
        plot_index += 1

        plt.xlabel('Number of gene families in the ancestral synteny')
        plt.ylabel('Time (s)')
        plt.boxplot(durations, positions=lengths)

plt.tight_layout(h_pad=3)
plt.savefig(os.path.join(cur_dir, 'viz.pdf'))

#!/usr/bin/env python3

import argparse
import json
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Plot the results from a sampled '
        + 'simulated evaluation of the algorithm.')
    parser.add_argument('input', action='store',
        help='input JSON file from which to read data')
    parser.add_argument('kind', action='store',
        help='kind of data to plot on the y-axis')
    parser.add_argument('output', action='store',
        help='path in which to create the output plot')

    args = parser.parse_args()

    ylabels = {
        'scoredif': 'DL-score difference from reference to reconciled tree',
        'duration': 'Time to compute reconciliation (s)'}

    # Plot configuration
    plt.rc('text', usetex=True)

    with open(args.input, 'r') as in_file:
        plt.xlabel('Number of gene families in the ancestral synteny')
        plt.ylabel(ylabels[args.kind])

        data = json.load(in_file)
        positions = []
        values = []

        for key, value in data.items():
            positions.append(int(key))

            if isinstance(value, dict):
                values.append(value[args.kind])
            else:
                values.append(value)

    # # Fit an exponential function to the average durations
    # avg_durations = list(map(np.average, durations))
    # exp = lambda x, a, b : a * np.exp(x * b)
    # [params, _] = opt.curve_fit(exp, lengths, avg_durations)

    # x = np.linspace(1, 11, 256, endpoint=True)
    # y = list(map(lambda x : exp(x, *params), x))

    # plt.plot(
    #     x, y,
    #     linestyle='dashed', linewidth=1,
    #     label=r'Best fit exponential'.format(*params))

    box = plt.boxplot(values, positions=positions)

    # plt.legend()
    plt.savefig(args.output)

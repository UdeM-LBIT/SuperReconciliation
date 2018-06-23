#!/usr/bin/env python3

import argparse
import json
import numpy as np
import scipy.optimize as opt
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser(description='Plot the results from a '
    + 'sampled simulated evaluation of the algorithm.')

parser.add_argument('input', action='store',
    help='input JSON file from which to read data')
parser.add_argument('x', action='store',
    help='kind of data on the x-axis')
parser.add_argument('y', action='store',
    help='kind of data on the y-axis')
parser.add_argument('output', action='store',
    help='path in which to create the output plot')

parser.add_argument('--log-scale', '-l', action='store_true',
    help='create a log scale')
parser.add_argument('--semilog-x', '-X', action='store_true',
    help='create a semilog scale in x')
parser.add_argument('--semilog-y', '-Y', action='store_true',
    help='create a semilog scale in y')
parser.add_argument('--fit-poly', action='store', type=int, default=0,
    help='fit a polynomial function of given degree to the data')
parser.add_argument('--fit-log', action='store_true',
    help='fit a logarithmic function to the data')
parser.add_argument('--fit-exp', action='store_true',
    help='fit an exponential function to the data')

args = parser.parse_args()

xlabels = {
    'synteny_size': 'Number of gene families in the ancestral synteny',
    'event_depth': 'Depth of the simulated tree'}

ylabels = {
    'scoredif': 'DL-score difference from reference to reconciled tree',
    'duration': 'Time to compute reconciliation (seconds)'}

# Plot configuration
plt.rc('text', usetex=True)

# Load data from file
with open(args.input, 'r') as in_file:
    plt.xlabel(xlabels[args.x])
    plt.ylabel(ylabels[args.y])

    data = json.load(in_file)
    positions = []
    values = []

    for value in data:
        positions.append(value['params'][args.x])
        value_normalized = value[args.y]

        # Convert from μs to seconds
        if args.y == 'duration':
            value_normalized = list(map(lambda x : x / 1e6, value_normalized))

        values.append(value_normalized)

# Fit an polynomial function to the average values
avg_values = list(map(np.average, values))

if args.fit_poly > 0:
    poly = np.polyfit(positions, avg_values, args.fit_poly)
    func = np.poly1d(poly)

    x = np.linspace(min(positions), max(positions), 256, endpoint=True)
    y = func(x)

    plt.plot(
        x, y,
        linestyle='dashed', linewidth=1,
        label=r'Best fit degree-{} polynomial'.format(args.fit_poly))
    plt.legend()

# Fit a logarithmic function to the average values
if args.fit_log:
    log = lambda x, a, b : a + b * np.log(x)
    [params, _] = opt.curve_fit(log, positions, avg_values)

    x = np.linspace(min(positions), max(positions), 256, endpoint=True)
    y = list(map(lambda x : log(x, *params), x))

    plt.plot(
        x, y,
        linestyle='dashed', linewidth=1,
        label=r'Best fit logarithm')
    plt.legend()

# Fit an exponential function to the average values
if args.fit_exp:
    exp = lambda x, a, b : a * np.exp(x * b)
    [params, _] = opt.curve_fit(exp, positions, avg_values)

    x = np.linspace(min(positions), max(positions), 256, endpoint=True)
    y = list(map(lambda x : exp(x, *params), x))

    plt.plot(
        x, y,
        linestyle='dashed', linewidth=1,
        label=r'Best fit exponential')
    plt.legend()

# Enable log scales
if args.semilog_x or args.log_scale:
    plt.xscale('log')

if args.semilog_y or args.log_scale:
    plt.yscale('log')

box = plt.boxplot(values, positions=positions)
plt.savefig(args.output)

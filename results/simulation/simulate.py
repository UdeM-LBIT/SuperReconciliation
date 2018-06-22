#!/usr/bin/env python3

import argparse
import json
from src.sample import sample

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Evaluate metrics of a sample of simulated evolutions '
            + 'for each value of a parameter over a given range.')

    required_group = parser.add_argument_group('required arguments')
    required_group.add_argument('output', action='store',
        help='path in which to create the output JSON file')
    required_group.add_argument('-m', '--metrics', action='append',
        choices=['scoredif', 'duration'], help='the metrics to evaluate',
        required=True)

    optional_group = parser.add_argument_group('optional arguments')
    optional_group.add_argument('-S', '--sample-size',
        action='store', type=int, default=500,
        help='size of the sample to take for each range value')
    optional_group.add_argument('-n', '--param-name',
        action='store', default='length',
        help='name of the variable simulation parameter')
    optional_group.add_argument('-v', '--param-values',
        action='store', default='1,5',
        help='list of values for the parameter to take, as a list of arguments '
            + 'passed to the `range` built-in')
    optional_group.add_argument('-j', '--jobs',
        action='store', type=int, default=0,
        help='number of parallel processes to use for the computation (if '
            + '0, uses one process per available core)')

    args = parser.parse_args()

    result = sample(
        sample_size=args.sample_size,
        param_name=args.param_name,
        param_values=range(*list(map(int, args.param_values.split(',')))),
        jobs=args.jobs,
        metrics=args.metrics)

    with open(args.output, 'w') as out_file:
        json.dump(result, out_file)

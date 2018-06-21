#!/usr/bin/env python3

import argparse
import json
from src.sample import sample

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description='Perform an evaluation of a sample of simulated evolutions '
            + 'for each value of a parameter over a given range.')
    parser.add_argument('output', action='store',
        help='path in which to create the output JSON file')
    parser.add_argument('-S', '--sample-size',
        action='store', type=int, default=500,
        help='size of the sample to take for each range value')
    parser.add_argument('-n', '--param-name',
        action='store', default='length',
        help='name of the variable simulation parameter')
    parser.add_argument('-v', '--param-values',
        action='store', default='1,5',
        help='list of values for the parameter to take, as a list of arguments '
            + 'passed to the `range` built-in')
    parser.add_argument('-j', '--jobs',
        action='store', type=int, default=0,
        help='number of parallel processes to use for the computation (if '
            + '0, uses one process per available core)')
    parser.add_argument('-m', '--metric',
        action='store', default='duration',
        help='the metric to evaluate, either \'scoredif\' or \'duration\'')

    args = parser.parse_args()

    result = sample(
        sample_size=args.sample_size,
        param_name=args.param_name,
        param_values=range(*list(map(int, args.param_values.split(',')))),
        jobs=args.jobs,
        kind=args.metric)

    with open(args.output, 'w') as out_file:
        json.dump(result, out_file)

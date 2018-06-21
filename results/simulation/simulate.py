#!/usr/bin/env python3

import argparse
import json
from src.sample import sampleParameterizedSimulation as sample

parser = argparse.ArgumentParser(
    description='Perform an evaluation of a sample of simulated evolutions '
        + 'for each value of a parameter over a given range.')
parser.add_argument('output', action='store',
    help='path in which to create the output JSON file')
parser.add_argument('-S', '--sample-size',
    action='store', type=int, default=500,
    help='size of the sample to take for each range value')
parser.add_argument('-p', '--ranged-param',
    action='store', default='length',
    help='name of the simulation parameter to range over')
parser.add_argument('-v', '--range-value',
    action='store', default='1,5',
    help='range to use for the parameter, as a list of arguments '
        + 'passed to the `range` function')

args = parser.parse_args()

result = sample(
    sample_size=args.sample_size,
    ranged_param=args.ranged_param,
    range_value=range(*list(map(int, args.range_value.split(',')))))

with open(args.output, 'w') as out_file:
    json.dump(result, out_file)

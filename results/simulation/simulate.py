#!/usr/bin/env python3

import os
import time
import collections

import functools
import json
import zss
import numpy as np

import interface

def computeDLScore(tree):
    """Compute the duplication-loss score of a tree."""
    root_cost = 0

    if 'event' in tree.features\
            and (tree.event == 'duplication' or tree.event == 'loss'):
        root_cost = 1

    return root_cost + functools.reduce(
        lambda x, y : x + y,
        map(computeDLScore, tree.children),
        0)

EvaluationResult = collections.namedtuple(
    'EvaluationResult',
    ['scoredif', 'duration'])

def simulateAndEvaluate(*args, **kwargs):
    """
    Simulate an evolution from a synteny, generate a synteny tree, and evaluate:
        – the time taken to reconcile the erased tree,
        — the difference between the original DL score and the reconciled score.

    :param *: same arguments as `simulate`
    :returns: a named tuple containing the DL score difference
        and the duration for which the reconciliation ran.
    """
    [original, erased] = interface.simulate(*args, **kwargs)
    start_time = time.perf_counter()
    reconciled = interface.reconcile(erased)
    end_time = time.perf_counter()

    original_score = computeDLScore(original)
    reconciled_score = computeDLScore(reconciled)

    if original_score < reconciled_score:
        raise Exception('Unexpected non-parcimonious reconciliation!')

    return EvaluationResult(
        scoredif=computeDLScore(original) - computeDLScore(reconciled),
        duration=end_time - start_time)

def sampleParameterizedSimulation(
    sample_size,
    ranged_param,
    range_value,
    **kwargs):
    """
    Perform an evaluation of a sample of simulated evolutions for each
    value of a parameter over a given range.

    :param sample_size: size of the sample to take.
    :param ranged_param: name of the parameter to range over.
    :param range_value: range of values for the parameter to take.
    :param *: remaining arguments are forwarded to `simulateAndEvaluate` as is.
    :return: a dictionary containing, for each value of the range, a named
        tuple containing the result of the evaluation for each sample.
    """
    result = {}

    for value in range_value:
        evaluation = EvaluationResult(
            scoredif=[],
            duration=[])

        for i in range(0, sample_size):
            kwargs[ranged_param] = value
            sample_evaluation = simulateAndEvaluate(**kwargs)

            evaluation.scoredif.append(sample_evaluation.scoredif)
            evaluation.duration.append(sample_evaluation.duration)

            print('> sample {} / {} for {} {} / {}{}'.format(
                i + 1, sample_size, ranged_param,
                value, max(range_value), ' ' * 20), end='\r')

        result[value] = evaluation

    return result

if __name__ == '__main__':
    cur_dir = os.path.dirname(os.path.abspath(__file__))
    result = sampleParameterizedSimulation(
        sample_size=5,
        ranged_param='length',
        range_value=range(1, 11))

    with open(os.path.join(cur_dir, 'simulation.json'), 'w') as out_file:
        json.dump(result, out_file)

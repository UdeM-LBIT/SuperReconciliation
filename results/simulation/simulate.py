#!/usr/bin/env python3

import subprocess
import os
import time
import collections

import json
import ete3
import zss
import numpy as np

cur_dir = os.path.dirname(os.path.abspath(__file__))

def parseNHX(intext):
    """Parse a NHX-formatted event tree using ete3"""
    return ete3.Tree(
        intext,
        format=1,
        quoted_node_names=True)

def stringifyNHX(intree):
    """Encode an ete3 event tree to a NHX-formatted string"""
    return intree.write(
        format=1,
        quoted_node_names=True,
        format_root_node=True,
        features=['event'])

SimulationResult = collections.namedtuple(
    'SimulationResult',
    ['original', 'erased'])

def simulate(
    seed=0,
    length=5,
    event_depth=5,
    duplication_probability=0.5,
    loss_probability=0.2,
    loss_length_rate=0.5):
    """
    Simulate evolution starting from a synteny of given length.

    :param seed: seed for the pseudo-random number generator used to simulate
        evolution. If 0, a random number given by the system will be used, if
        available.
    :param length: length of the ancestral synteny (number of gene families)
        to be used as a basis for the simulation.
    :param event_depth: maximum depth of the tree in terms of duplication and
        speciation events.
    :param loss_probability: probability for a loss to occur under any given
        node of the tree.
    :param loss_length_rate: parameter for the geometric distribution used to
        get a loss’ length in terms of number of genes lost.
    :returns: a named tuple containing the original tree describing the
        simulated evolution and an erased version of the tree where loss events
        have been removed alongwith internal non-root synteny labelling.
    """
    process = subprocess.Popen(
        [
            os.path.join(cur_dir, '../../build/Release/simulate'),
            str(seed), str(length), str(event_depth),
            str(duplication_probability), str(loss_probability),
            str(loss_length_rate)
        ],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    [data, error] = process.communicate()
    [original, erased, error] = data.decode().split('\n')

    if process.returncode != 0:
        raise Exception(error.decode())

    return SimulationResult(parseNHX(original), parseNHX(erased))

def reconcile(intree):
    """
    Reconcile a synteny tree, labeling the internal nodes with syntenies
    and introducing loss events where necessary, so as to minimize the
    number of duplications and losses.

    :param intree: input tree as an ete3 structure.
    :returns: reconciled tree.
    """
    process = subprocess.Popen(
        [os.path.join(cur_dir, '../../build/Release/super_reconciliation')],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    intext = stringifyNHX(intree).encode('utf-8')
    [data, error] = process.communicate(intext)
    outtext = data.decode()

    if process.returncode != 0:
        raise Exception(error.decode())

    return parseNHX(outtext)

def editDistance(before, after):
    """
    Compute the edit distance between two trees.

    :param before: first tree to compare.
    :param after: second tree to compare.
    :returns: the edit distance between before and after.
    """
    def getLabel(node):
        return node.name

    def getChildren(node):
        return node.children

    def labelDist(labelA, labelB):
        if labelA == labelB:
            return 0
        else:
            return 1

    return zss.simple_distance(
        before,
        after,
        getChildren,
        getLabel,
        labelDist)

EvaluationResult = collections.namedtuple(
    'EvaluationResult',
    ['distance', 'time'])

def simulateAndEvaluate(*args, **kwargs):
    """
    Run a simulation and evaluate the distance from the original evolution
    tree to the reconciled erased tree.

    :param *: same arguments as `simulate`
    :returns: a named tuple containing the edit distance and the time
        taken for reconciliation.
    """
    [original, erased] = simulate(*args, **kwargs)
    start_time = time.perf_counter()
    reconciled = reconcile(erased)
    end_time = time.perf_counter()

    return EvaluationResult(
        editDistance(original, reconciled),
        end_time - start_time)

SampleResult = collections.namedtuple(
    'SampleResult',
    ['distances', 'durations'])

def sampledEvaluation(
    sample_size,
    lengths,
    *args, **kwargs):
    """
    Evaluate simulation distance and reconciliation time for a given set of
    parameters over various synteny lengths.

    :param sample_size: size of the sample to take for each length.
    :param lengths: lengths of the syntenies to test.
    :param *: remaining arguments are forwarded to `simulateAndEvaluate` as is.
    :return: a named tuple containing the set of distance and time results
        for each length.
    """
    overall_distances = []
    overall_durations = []

    for length in lengths:
        current_distances = []
        current_durations = []

        for i in range(0, sample_size):
            [distance, duration] = simulateAndEvaluate(
                length=length,
                *args, **kwargs)

            current_distances.append(distance)
            current_durations.append(duration)

            print('> ' + str(i + 1) + '/' + str(sample_size) + ' in length '
                    + str(length) + '                      ', end='\r')

        overall_distances.append(current_distances)
        overall_durations.append(current_durations)

    print()
    return SampleResult(overall_distances, overall_durations)

if __name__ == '__main__':
    sample_size = 5
    lengths = range(1, 11)
    loss_probs = np.linspace(0, 1, 11)

    for loss_prob in loss_probs:
        # Prevent from showing unsignificant digits
        display_prob = "{:.15g}".format(loss_prob)

        print('Evaluation for loss_prob = ' + display_prob)
        out_dir = os.path.join(cur_dir, 'p-' + display_prob)

        try:
            os.mkdir(out_dir)
        except OSError:
            pass

        [distances, durations] = sampledEvaluation(
            sample_size=sample_size,
            lengths=lengths,
            loss_probability=loss_prob)

        with open(os.path.join(out_dir, 'distances.json'), 'w') as out_file:
            json.dump(distances, out_file)

        with open(os.path.join(out_dir, 'durations.json'), 'w') as out_file:
            json.dump(durations, out_file)

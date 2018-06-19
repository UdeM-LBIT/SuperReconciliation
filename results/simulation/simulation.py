#!/usr/bin/env python3

from subprocess import Popen, PIPE
import time

from ete3 import Tree
import zss

import numpy
import matplotlib.pyplot as plt

def parseNHX(intext):
    return Tree(intext, format=1, quoted_node_names=True)

def stringifyNHX(intree):
    return intree.write(format=1, quoted_node_names=True,
        format_root_node=True, features=['event'])

def simulate(
    seed=0,
    length=5,
    event_depth=5,
    duplication_probability=0.5,
    loss_probability=0.8,
    loss_length_rate=0.5):

    pipe = Popen([
        '../../build/Release/simulate',
        str(seed), str(length), str(event_depth),
        str(duplication_probability), str(loss_probability),
        str(loss_length_rate)
    ], stdin=PIPE, stdout=PIPE, stderr=PIPE)

    [original, erased, _] = pipe.communicate()[0].decode().split('\n')
    return [parseNHX(original), parseNHX(erased)]

def reconcile(intree):
    pipe = Popen(['../../build/Release/super_reconciliation'],
            stdin=PIPE, stdout=PIPE, stderr=PIPE)

    intext = stringifyNHX(intree).encode('utf-8')
    outtext = pipe.communicate(intext)[0].decode()
    return parseNHX(outtext)

def editDistance(before, after):
    def getLabel(node):
        return node.name

    def getChildren(node):
        return node.children

    def labelDist(labelA, labelB):
        if labelA == labelB:
            return 0
        else:
            return 1

    return zss.simple_distance(before, after, getChildren, getLabel, labelDist)

def simulateAndEvaluate(*args, **kwargs):
    [original, erased] = simulate(*args, **kwargs)
    start_time = time.perf_counter()
    reconciled = reconcile(erased)
    end_time = time.perf_counter()
    return [editDistance(original, reconciled), end_time - start_time]

distances = []
durations = []

lengths = range(1, 11)
sample = 500

for length in lengths:
    current_distances = []
    current_durations = []

    for i in range(0, sample):
        [distance, duration] = simulateAndEvaluate(length=length)
        current_distances.append(distance)
        current_durations.append(duration)
        print(length, ':', i, 'of', sample, end='\r')

    distances.append(current_distances)
    durations.append(current_durations)

print('distances :', distances)
print('durations :', durations)

plot_distances = plt.figure(1)
plt.title('Distance from the reference to the reconciled tree')
plt.xlabel('Number of gene families in the ancestral synteny')
plt.ylabel('Edit distance')
plt.boxplot(distances, positions=lengths)
plot_distances.show()

plot_durations = plt.figure(2)
plt.title('Reconciled tree computation time')
plt.xlabel('Number of gene families in the ancestral synteny')
plt.ylabel('Time (s)')
plt.boxplot(durations, positions=lengths)
plot_durations.show()

input()

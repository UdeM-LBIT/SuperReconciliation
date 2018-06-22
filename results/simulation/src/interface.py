"""
Utility functions interfacing with the C++ programs under test.
"""

import collections as _collections

SimulationResult = _collections.namedtuple(
    'SimulationResult',
    ['original', 'erased'])

def parseNHX(intext):
    """Parse a NHX-formatted event tree using ete3"""
    import ete3
    return ete3.Tree(
        intext,
        format=1,
        quoted_node_names=True)

def stringifyNHX(intree):
    """Encode an ete3 event tree to a NHX-formatted string"""
    import ete3
    return intree.write(
        format=1,
        quoted_node_names=True,
        format_root_node=True,
        features=['event'])

def simulate(
    seed=0,
    length=10,
    event_depth=5,
    duplication_probability=0.5,
    loss_probability=0.5,
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
    import subprocess
    import os

    cur_dir = os.path.dirname(os.path.abspath(__file__))
    process = subprocess.Popen(
        [
            os.path.join(cur_dir, '../../../build/Release/simulate'),
            str(seed), str(length), str(event_depth),
            str(duplication_probability), str(loss_probability),
            str(loss_length_rate)
        ],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    [data, error] = process.communicate()
    [original, erased, _] = data.decode().split('\n')

    if process.returncode != 0:
        raise Exception('Subprocess terminated abnormally.\n\n'
            + 'stdin={}\n\nstdout={}'.format(data.decode(), error.decode()))

    return SimulationResult(parseNHX(original), parseNHX(erased))

def reconcile(intree):
    """
    Reconcile a synteny tree, labeling the internal nodes with syntenies
    and introducing loss events where necessary, so as to minimize the
    number of duplications and losses.

    :param intree: input tree as an ete3 structure.
    :returns: reconciled tree.
    """
    import subprocess
    import os

    cur_dir = os.path.dirname(os.path.abspath(__file__))
    process = subprocess.Popen(
        [os.path.join(cur_dir, '../../../build/Release/super_reconciliation')],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)

    intext = stringifyNHX(intree).encode('utf-8')
    [data, error] = process.communicate(intext)
    outtext = data.decode()

    if process.returncode != 0:
        raise Exception('Subprocess terminated abnormally.\n\n'
            + 'stdin={}\n\nstdout={}'.format(data.decode(), error.decode()))

    return parseNHX(outtext)

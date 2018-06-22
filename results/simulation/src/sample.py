def computeDLScore(tree):
    """Compute the duplication-loss score of a tree."""
    import functools
    root_cost = 0

    if 'event' in tree.features\
            and (tree.event == 'duplication' or tree.event == 'loss'):
        root_cost = 1

    return root_cost + functools.reduce(
        lambda x, y : x + y,
        map(computeDLScore, tree.children),
        0)

def simulateAndEvaluate(metrics, **kwargs):
    """
    Simulate an evolution from a synteny, generate a synteny tree, and evaluate
    either:
        – the time taken to reconcile the erased tree ('duration'),
        — the difference between the original DL score and the reconciled score
          ('scoredif').

    :param metrics: a list of metrics to evaluate, either 'scoredif'
        or 'duration'.
    :param *: remaining arguments are forwarded to `simulate` as is.
    :returns: evaluation result.
    """
    from . import interface
    import time

    [original, erased] = interface.simulate(**kwargs)
    start_time = time.perf_counter()
    reconciled = interface.reconcile(erased)
    end_time = time.perf_counter()

    result = {}

    if metrics.count('scoredif') >= 1:
        original_score = computeDLScore(original)
        reconciled_score = computeDLScore(reconciled)

        if original_score < reconciled_score:
            raise Exception('Unexpected non-parcimonious reconciliation!')

        result['scoredif'] = original_score - reconciled_score

    if metrics.count('duration') >= 1:
        result['duration'] = end_time - start_time

    return result

def _evaluateSample(param_value, sample_size, param_name, args):
    from functools import partial

    local_args = args.copy()
    local_args[param_name] = param_value

    return (param_value, list(map(
        lambda _ : simulateAndEvaluate(**local_args),
        range(0, sample_size))))

def sample(
    sample_size,
    param_name,
    param_values,
    jobs,
    **kwargs):
    """
    Perform an evaluation of a sample of simulated evolutions for each
    value of a parameter over a given range.

    :param sample_size: size of the sample to take for each range value.
    :param param_name: name of the variable simulation parameter.
    :param param_values: list of values for the parameter to take.
    :param jobs: number of parallel processes to use for the computation (if 0,
        uses one process per available core).
    :param *: remaining arguments are forwarded to `simulateAndEvaluate` as is.
    :return: a dictionary containing, for each value of the range, a list of
        results for each metric for each sample.
    """
    import multiprocessing
    from functools import partial
    import sys

    if jobs == 0:
        jobs = multiprocessing.cpu_count()

    pool = multiprocessing.Pool(jobs)
    results = []
    message = '\r[{:%}] {}/{} {} values evaluated'

    print('> Using {} process(es) to compute'.format(jobs))
    print(message.format(0, 0, len(param_values), param_name),
        flush=True, end='')

    for i, result in enumerate(pool.imap_unordered(
            partial(_evaluateSample,
                sample_size=sample_size,
                param_name=param_name,
                args=kwargs),
            param_values)):
        print(message.format(
            (i + 1) / len(param_values), i + 1,
            len(param_values), param_name),
            flush=True, end='')

        results.append(result)

    print()
    return dict(results)

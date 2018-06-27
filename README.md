# SuperReconciliation

Implementation of the Super-Reconciliation model for reconciling a set of trees accounting for segmental duplications and losses.

_Note:_ for now, this implementation only works on supertrees and still lacks functionality to build supertrees from a set of consistent gene trees.

## Building

### Requirements

* a C++ compiler supporting the C++14 standard, _eg._ Clang ≥3.4 or GCC ≥6;
* Boost libraries ≥1.60;
* CMake ≥3.1.

> **Warning:** there is a bug in GCC preventing the `evaluation` program from working properly. This bug was reported and fixed in releases starting from 6.5, 7.4, 8.2 or 9 (see [the bug report](https://gcc.gnu.org/bugzilla/show_bug.cgi?id=86291) for more details).

### Commands

The following commands can be used for building on a Linux-based machine meeting the requirements:

```sh
mkdir -p build/Release
cd build/Release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

## Usage

After building, all executables can be found in `build/Release`. To ensure everything works as intended, run the unit tests program, `tests`, and make sure that all tests pass (feel free to report any problem).

### Input/output formats

The various programs use [the NHX format](https://home.cc.umanitoba.ca/~psgendb/doc/atv/NHX.pdf) for inputting or outputting trees, in which the name of a node is used for encoding its synteny (list of gene families) and the NHX tag `event` is used for encoding the event (either `duplication`, `speciation` or `loss`) at a given node.

For example, the following NHX string represents one of the erased synteny trees of the paper (see the `examples` directory for more examples):

```nhx
(
    (
        "x x' x''",
        [&&NHX:event=loss]
    )[&&NHX:event=speciation],
    (
        "x",
        (
            "x x''",
            "x x'"
        )[&&NHX:event=duplication]
    )[&&NHX:event=speciation]
)"x x' x''"[&&NHX:event=duplication];
```

![Graphviz visualisation of the previous NHX string](examples/simple.in.png)

### Programs

#### `super_reconciliation`

This is the main program. It takes an erased supertree on standard input and outputs the inferred tree based on the Super-Reconciliation method. This implements the main algorithm of the paper.

#### `simulate`

Randomly simulate an evolutionary history based on a ficticious ancestral synteny of given length, and outputs a fully-labeled tree of this history.

```
Usage: ./simulate [-h] [options...]

Simulate the evolution of a ficticious synteny.

General options:
  -h [ --help ]                    show this help message

Simulation parameters:
  -s [ --synteny-size ] SIZE (=5)  size of the ancestral synteny to evolve from
  -d [ --depth ] SIZE (=5)         maximum depth of events on a branch, not
                                   counting losses
  -D [ --p-dup ] PROB (=0.5)       probability for any given internal node to
                                   be a duplication
  -L [ --p-loss ] PROB (=0.5)      probability for a loss under any given
                                   speciation node
  -R [ --p-length ] PROB (=0.5)    parameter defining the geometric
                                   distribution of loss segments’ lengths
```

#### `erase`

Erase information from a full synteny tree to make it suitable for super reconciliation.

#### `evaluate`

Create a sample of simulated evolutions, and, for each reference tree, erase information and use the result as input to the Super-Reconciliation algorithm. Evaluate given metrics:

* `dlscore`: difference between the reference tree’s duplication-loss count and the reconciled tree’s duplication-loss count;
* `duration`: measure the time required to compute the Super-Reconciliation.

```
Usage: ./evaluate [-h] output -m METRIC [options...]

Evaluate metrics of a sample of evolutions simulated for each
given set of parameters.

Required arguments:
  -o [ --output ] PATH             path in which to create the output file
  -m [ --metrics ] METRIC          the metrics to evaluate, either 'dlscore' or
                                   'duration'

General options:
  -h [ --help ]                    show this help message
  -S [ --sample-size ] SIZE (=1)   number of samples to take for each set of
                                   parameters
  -j [ --jobs ] JOBS (=0)          number of threads to use for computing. If
                                   0, automatically evaluate the best amount of
                                   threads based on the resources of the
                                   machine. Set to 1 to disable multithreading

Simulation parameters (accept either a single value, a set of values '{1, 2, 3}'
or a range of values '[1:100]' with an optional step argument '[1:100:10]'):
  -s [ --synteny-size ] SIZE (=5)  size of the ancestral synteny to evolve from
  -d [ --depth ] SIZE (=5)         maximum depth of events on a branch, not
                                   counting losses
  -D [ --p-dup ] PROB (=0.5)       probability for any given internal node to
                                   be a duplication
  -L [ --p-loss ] PROB (=0.5)      probability for a loss under any given
                                   speciation node
  -R [ --p-length ] PROB (=0.5)    parameter defining the geometric
                                   distribution of loss segments’ lengths
```

#### `viz`

Generate a visualization of a synteny tree. Takes a synteny tree on standard input and outputs it in a Graphviz-compatible format on standard output. If you pipe the output to the `dot` utility, you can view the tree in a variety of formats such as PNG or PDF.

### `tests`

Run unit tests.

### Example

Simulate one evolutionary history and output three trees:

* the reference, fully-labeled tree;
* the erased version with removed labels on internal nodes;
* the reconciled version after applying the Super-Reconciliation method.

```sh
./simulate | tee
    >(./erase | tee
        >(./viz | dot -Tpdf >! tree-erased.pdf)
        >(./super_reconciliation | ./viz | dot -Tpdf >! tree-reconciled.pdf)
    )
    | ./viz | dot -Tpdf >! tree-reference.pdf
```

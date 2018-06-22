# SuperReconciliation

## Input/output formats

The various programs use [the NHX format](https://home.cc.umanitoba.ca/~psgendb/doc/atv/NHX.pdf) for inputting or outputting trees, in which the name of a node is used for encoding its synteny (list of gene families) and the NHX tag `event` is used for encoding the event (either `duplication`, `speciation` or `loss`) at a given node.

For example, the following NHX string represents one of the erased synteny trees of the paper (see the `examples/` directory for more examples):

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

## Main utilities

The main utilities are implemented using C++14 and use CMake as their build system.

### Build

Requirements for building are:

* Boost libraries ≥1.60;
* CMake ≥3.1;
* a C++ compiler supporting the C++14 standard, _eg._ Clang ≥3.4 or GCC ≥5.

The following sequence of commands can be used for building on a Linux-based machine meeting the requirements:

```sh
mkdir -p build/Release
cd build/Release
cmake -DCMAKE_BUILD_TYPE=Release ../..
make
```

All resulting executables are located in `build/Release`. To ensure everything works as intended, run the unit tests executable named `tests` and make sure all tests pass (feel free to report any problem).

### Run

* `super_reconciliation`: main program. Takes a erased tree on standard input and outputs its reconciled version on standard output. This implements the main algorithm of the paper.
* `tests`: unit tests.
* `simulate`: simulate the evolution of a synteny. Outputs the full version of the resulting tree followed by the erased version on the next line.
* `viz`: visualization of a synteny tree. Takes a synteny tree on standard input and outputs it in a Graphviz-compatible format on standard output. If you pipe the output to the `dot` utility, you can view the tree in a variety of formats such as PNG or PDF.
* `erase`: erase information from a full synteny tree to make it suitable for super reconciliation.

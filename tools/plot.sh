#!/bin/bash

# Generate the plots used in the paper

./plot.py results/ordered-by-depth.json \
    depth duration \
    output/ordered-time-by-depth.pdf
./plot.py results/ordered-by-depth.json \
    depth duration \
    output/ordered-time-by-depth.pgf

./plot.py results/ordered-by-length.json \
    base_size duration \
    output/ordered-time-by-length.pdf \
    --fit-exp --semilog-y
./plot.py results/ordered-by-length.json \
    base_size duration \
    output/ordered-time-by-length.pgf \
    --fit-exp --semilog-y

./plot.py results/unordered-by-length.json \
    base_size duration \
    output/unordered-time-by-length.pdf \
    --kind average
./plot.py results/unordered-by-length.json \
    base_size duration \
    output/unordered-time-by-length.pgf \
    --kind average

#!/bin/bash

./plot.py results/by-depth.json \
    event_depth duration \
    output/simulation-time-by-depth.pdf \
    --fit-poly=4
./plot.py results/by-depth.json \
    event_depth duration \
    output/simulation-time-by-depth.pgf \
    --fit-poly=4

./plot.py results/by-length.json \
    synteny_size duration \
    output/simulation-time-by-length.pdf \
    --fit-exp --semilog-y
./plot.py results/by-length.json \
    synteny_size duration \
    output/simulation-time-by-length.pgf \
    --fit-exp --semilog-y

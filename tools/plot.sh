#!/bin/bash

./plot.py results/by-depth.json \
    event_depth duration \
    output/simulation-time-by-depth.pdf
./plot.py results/by-depth.json \
    event_depth duration \
    output/simulation-time-by-depth.pgf

./plot.py results/by-depth.json \
    event_depth dlscore \
    output/simulation-dlscore-by-depth.pdf
./plot.py results/by-depth.json \
    event_depth dlscore \
    output/simulation-dlscore-by-depth.pgf

./plot.py results/by-length.json \
    synteny_size duration \
    output/simulation-time-by-length.pdf \
    --fit-exp --semilog-y
./plot.py results/by-length.json \
    synteny_size duration \
    output/simulation-time-by-length.pgf \
    --fit-exp --semilog-y

./plot.py results/by-length.json \
    synteny_size dlscore \
    output/simulation-dlscore-by-length.pdf
./plot.py results/by-length.json \
    synteny_size dlscore \
    output/simulation-dlscore-by-length.pgf

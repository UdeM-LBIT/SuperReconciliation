#!/bin/bash

./plot.py output/by-length.json \
    synteny_size dlscore \
    output/simulation-score-by-length.pdf
./plot.py output/by-length.json \
    synteny_size dlscore \
    output/simulation-score-by-length.pgf

./plot.py output/by-length.json \
    synteny_size duration \
    output/simulation-time-by-length.pdf \
    --fit-exp --semilog-y
./plot.py output/by-length.json \
    synteny_size duration \
    output/simulation-time-by-length.pgf \
    --fit-exp --semilog-y

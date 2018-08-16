#!/usr/bin/env bash

set -x

SIMULFLAGS="-R 0.75"
ERASEFLAGS=
RECONFLAGS="--unordered"

function makeviz
{
    echo "$1" | ./viz | dot -Tpdf > "$2"
}

reference="$(./simulate $SIMULFLAGS)"
echo "$reference" > tree-reference.nhx
makeviz "$reference" tree-reference.pdf

erased="$(echo "$reference" | ./erase $ERASEFLAGS)"
echo "$erased" > tree-erased.nhx
makeviz "$erased" tree-erased.pdf

reconciled="$(echo "$erased" | ./reconcile $RECONFLAGS)"
echo "$reconciled" > tree-reconciled.nhx
makeviz "$reconciled" tree-reconciled.pdf

#!/usr/bin/env bash

build="../build"
dir="simulation/$RANDOM"
mkdir -p "$dir"

echo "--- Generating a simulated tree ---"
echo "Directory: $dir"
simulation_results="$("$build/simulate")"
echo ""

original="$(echo "$simulation_results" | head -1)"
echo "$original" > "$dir/original"
"$build/viz" < "$dir/original" | dot -Tpdf > "$dir/original.pdf"

erased="$(echo "$simulation_results" | tail -1)"
echo "$erased" > "$dir/erased"
"$build/viz" < "$dir/erased" | dot -Tpdf > "$dir/erased.pdf"

"$build/super_reconciliation" < "$dir/erased" > "$dir/reconciled"
"$build/viz" < "$dir/reconciled" | dot -Tpdf > "$dir/reconciled.pdf"

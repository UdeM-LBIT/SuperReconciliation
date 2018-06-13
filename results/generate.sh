#!/usr/bin/env bash

build="$1"
src="$2"
dest_in="$src.in.pdf"
dest_out="$src.out.pdf"

"$build/viz" < "$src" | dot -Tpdf > "$dest_in"
"$build/main" < "$src" | "$build/viz" | dot -Tpdf > "$dest_out"

xdg-open "$dest_in"
xdg-open "$dest_out"

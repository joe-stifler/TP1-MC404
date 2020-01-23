#!/bin/bash
for var in {0..9}
    do
        ./main arq0$var.in >out.out
        diff arq0$var.res out.out
    done

for var in {10..18}
    do
      ./main arq$var.in >out.out
      diff arq$var.res out.out
    done
exit 0

#!/bin/bash
tee >(stdbuf -i0 -o0 -e0 ./pmtest) | unbuffer -p tee >( ./pmstress) >( ./pmstress) >( ./pmstress) >( ./pmstress)


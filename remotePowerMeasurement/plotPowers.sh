#!/bin/bash

dir=$(dirname $0)

cmd="$dir/feedgnuplot/bin/feedgnuplot --line --stream 0.1 --xlen 120"

$dir/streamPowers.sh | tee >(stdbuf -oL -eL cut -d \  -f 3 | $cmd --legend 0 'whole cluster') | stdbuf -oL -eL cut -d \  -f 4,5,6,7,8 | $cmd --legend 0 'jjpowerserver' --legend 1 'jjpowerserver1' --legend 2 'jjpowerserver2' --legend 3 'jjpowerserver3' --legend 4 'jjpowerserver4'

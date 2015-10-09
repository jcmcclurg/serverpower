#!/bin/bash

dir=$(dirname $0)

$dir/streamPowers.sh | stdbuf -oL -eL cut -d \  -f 4,5,6,7,8 | $dir/feedgnuplot/bin/feedgnuplot --line --legend 0 'jjpowerserver' --legend 1 'jjpowerserver1' --legend 2 'jjpowerserver2' --legend 3 'jjpowerserver3' --legend 4 'jjpowerserver4' --stream 0.1 --xlen 120

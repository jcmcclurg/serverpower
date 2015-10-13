#!/bin/bash

dir=$( dirname $0 )
addr='http://192.168.1.200:8282'

$dir/stopStreaming.sh $addr

c=$[ 3*9 ]
l=$[ (10000*$c)/60]
s=$[ ($l*1000)/10000 ]
echo "Setting up the streaming to go as fast as is needed ($l-sample [aka $c-cycle, or $s millisecond] averages, as fast as they come in)"

s=$(wget -O - "$addr/stream?command=start&length=$l&blockLength=$l&type=power&fields=01234567" --quiet)
echo "Start stream returned ($s)"

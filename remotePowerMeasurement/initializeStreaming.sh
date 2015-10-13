#!/bin/bash

dir=$( dirname $0 )
addr='http://192.168.1.200:8282'

$dir/stopStreaming.sh $addr

echo "Setting up the streaming to go as fast as is prudent (1500-sample [aka 9-cycle] averages, as fast as they come in)"
echo "The format is timestamp,power of jjpowerserver"
echo "I did some tests to make sure that the cut command wasn't incurring any power penalty."

s=$(wget -O - "$addr/stream?command=start&length=1500&blockLength=1500&type=power&fields=14" --quiet)
echo "Start stream returned ($s)"

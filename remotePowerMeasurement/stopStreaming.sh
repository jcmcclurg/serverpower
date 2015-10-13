#!/bin/bash

dir=$( dirname $0 )
defAddr='http://192.168.1.200:8282'

addr=${1:-$defAddr}

s=$(wget -O - "$addr/stream?command=stop" --quiet)
echo "Stop stream returned ($s)"

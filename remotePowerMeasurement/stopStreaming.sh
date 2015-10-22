#!/bin/bash
# type 192.168.1200:8282/stream?command=info into browser to see active streams
dir=$( dirname $0 )
defAddr='http://192.168.1.200:8282'

addr=${1:-$defAddr}

s=$(wget -O - "$addr/stream?command=stop&address=224.1.1.1&port=9500" --quiet)
echo "Stop stream returned ($s)"
s=$(wget -O - "$addr/stream?command=stop&address=224.1.1.1&port=9501" --quiet)
echo "Stop stream returned ($s)"
echo "Check if any streams still running:"
wget -O - "$addr/stream?command=info" --quiet

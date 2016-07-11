#!/bin/bash

dir=$( dirname $0 )
defAddr='http://192.168.1.200:8282'
defGroup='224.1.1.1'
defPort=9990
addr=${1:-$defAddr}
group=${2:-$defGroup}
port=${3:-$defPort}

s=$(wget -O - "$addr/stream?command=stop&address=$group&port=$port" --quiet)
echo "Stop stream returned ($s)"

s=$(wget -O - "$addr/stream?command=start&length=10000&blockLength=1000&type=power&fields=01234567&delimiter=%20&address=$group&port=$port" --quiet)
echo "Start stream returned ($s)"

python -u $dir/measurementServer/multicast_listen.py -n -a $group -p $port | tee $dir/powerlog.log
#python -u $dir/measurementServer/multicast_listen.py -n -a $group -p $port | $cmd --legend 0 'jjpowerserver3' --legend 1 'jjpowerserver4'

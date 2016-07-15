#!/bin/bash

dir=$( dirname $0 )
defAddr='http://192.168.0.200:8282'
defGroup='224.1.1.1'
defPort=9995
addr=${1:-$defAddr}
group=${2:-$defGroup}
port=${3:-$defPort}

trap "wget -O - '$addr/stream?command=stop&address=$group&port=$port' --quiet" SIGINT SIGTERM

s=$(wget -O - "$addr/stream?command=stop&address=$group&port=$port" --quiet)
echo "Stop stream returned ($s)"

s=$(wget -O - "$addr/stream?command=start&length=10000&blockLength=500&type=power&fields=34567&delimiter=%20&address=$group&port=$port" --quiet)
echo "Start stream returned ($s)"

cmd="$dir/feedgnuplot/bin/feedgnuplot --line --stream 0.1 --xlen 120"

python -u $dir/measurementServer/multicast_listen.py -n -a $group -p $port | $cmd --legend 0 'jjpowerserver0' --legend 1 'jjpowerserver1' --legend 2 'jjpowerserver2' --legend 3 'jjpowerserver3' --legend 4 'jjpowerserver4'

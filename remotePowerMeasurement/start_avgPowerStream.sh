#!/bin/bash

dir=$( dirname $0 )
defAddr='http://192.168.1.200:8282'
defGroup='224.1.1.1'
defPort=9994
addr=${1:-$defAddr}
group=${2:-$defGroup}
port=${3:-$defPort}

# Field meanings
# 0 = start time
# 1 = end time
# 2 = total power
# 3 = jjpowerserver0
# 4 = jjpowerserver1
# 5 = jjpowerserver2
# 6 = jjpowerserver3
# 7 = jjpowerserver4

if [ -f $dir/.avgPowerStream ]; then
	echo "Stream already running." >&2
else
	s=$(wget -O - "$addr/stream?command=start&length=1000&blockLength=1000&type=power&fields=56&delimiter=%20&address=$group&port=$port" --quiet)
	echo "Start stream returned ($s)"

	echo $addr > $dir/.avgPowerStream
	echo $group >> $dir/.avgPowerStream
	echo $port >> $dir/.avgPowerStream
fi

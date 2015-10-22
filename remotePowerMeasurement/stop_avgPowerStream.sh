#!/bin/bash

dir=$( dirname $0 )

if [ -f $dir/.avgPowerStream ]; then
	lines=($( cat $dir/.avgPowerstream ))
	addr=${lines[0]}
	group=${lines[1]}
	port=${lines[2]}

	s=$(wget -O - "$addr/stream?command=stop&address=$group&port=$port" --quiet)
	echo "Stop stream returned ($s)"
	rm $dir/.avgPowerStream
else
	echo "Stream not started." >& 2
fi

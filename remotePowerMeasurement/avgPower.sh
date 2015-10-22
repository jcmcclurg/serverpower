#!/bin/bash

dir=$( dirname $0 )

if [ -f $dir/.avgPowerStream ]; then
	lines=($( cat $dir/.avgPowerStream ))
	addr=${lines[0]}
	group=${lines[1]}
	port=${lines[2]}

	python -u $dir/measurementServer/multicast_listen.py -n -a $group -p $port | sed -u "s/ \\+/+/g" | sed -u "s/^.*$/scale=3; (&)\\/2/g" | bc
else
	echo "Stream not started." >& 2
fi

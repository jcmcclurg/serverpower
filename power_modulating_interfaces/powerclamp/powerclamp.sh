#!/bin/bash

defDevice=$(dirname $(grep -l intel_powerclamp $(ls /sys/class/thermal/*/type) ) )
device=${1:-$defDevice}

rangeMax=`cat $device/max_state`
rangeMin=1

echo Welcome to the powerclamp cpu limiter. Please make sure you have >&2
echo the module loaded. Type the limiting values. Press CTRL+C or q to exit. >&2
echo "The range is $rangeMin-$rangeMax. The value is duty cycle in percent of work (100 - percent idle)." >&2

limitVal=0

trap "echo '0' > $device/cur_state; echo 'Reset powerclamp on exit.' >&2; exit;" SIGINT SIGTERM

while [[ "$limitVal" != "q" ]]; do
	if [ $(echo "$limitVal < $rangeMin" | bc) == 1 ]; then
		limitVal=$rangeMin
	elif [ $(echo "$limitVal > $rangeMax" | bc) == 1 ]; then
		limitVal=$rangeMax
	fi
	echo "1 + $rangeMax - $limitVal" | bc > $device/cur_state
	echo "Set to $limitVal" >&2
	read limitVal
done
echo "0" > $device/cur_state;
echo 'Reset powerclamp.' >&2;
echo "Done" >&2

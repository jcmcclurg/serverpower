#!/bin/bash

defDevice=$(dirname $(grep -l intel_powerclamp $(ls /sys/class/thermal/*/type) ) )
device=${1:-$defDevice}

rangeMax=`cat $device/max_state`
rangeMin=1

echo Welcome to the powerclamp cpu limiter. Please make sure you have >&2
echo the module loaded. Press CTRL+C or q to exit. >&2
echo "The range is $rangeMin to $rangeMax. The value is the percent of idle injection." >&2

val=0

trap "echo '0' > $device/cur_state; echo 'Reset powerclamp on exit.' >&2; exit;" SIGINT SIGTERM

while [[ "x$val" != "xq" ]]; do
	if [ $(echo "$val < $rangeMin" | bc) == 1 ]; then
		val=$rangeMin
	elif [ $(echo "$val > $rangeMax" | bc) == 1 ]; then
		val=$rangeMax
	fi
	echo $val > $device/cur_state
	cs=$(cat $device/cur_state)
	echo "Set to $cs." >&2
	read val
done

echo "0" > $device/cur_state;
echo 'Reset powerclamp.' >&2;
echo "Done" >&2

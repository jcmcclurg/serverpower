#!/bin/bash

sleeplen=15

freqVals=($(sudo cpufreq-info -s | sed -e "s/:[^\n,]\+, \+/\n/g" | sed -e "s/:.\+//g"))

sleep $sleeplen
len=${#freqVals[@]}
for i in $(seq $len); do
	d=${freqVals[$[ $i - 1 ]]}
	echo "Step to $d at "`date +%s.%N` >&2
	echo "$d"
	sleep $sleeplen

	d=${freqVals[$[ $len - $i ]]}
	echo "Step to $d at "`date +%s.%N` >&2
	echo "$d"
	sleep $sleeplen
done

echo "Quitting at "`date +%s.%N` >&2
echo "q"

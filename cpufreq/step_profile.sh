#!/bin/bash

sleeplen=15
steps=200
minPower=0
maxPower=50

freqVals=($(sudo cpufreq-info -s | sed -e "s/:[^\n,]\+, \+/\n/g" | sed -e "s/:.\+//g"))

sleep $sleeplen

for i in $(seq ${#freqVals[@]}); do
	d=`echo "($i - 1)/$steps + $minPower" | bc`
	echo "Step to $d at "`date +%s.%N` >&2
	printf "%0.3f\n" $d
	sleep $sleeplen

	d=`echo "scale=3; ($maxPower - $minPower)*($steps - $i)/$steps + $minPower" | bc`
	echo "Step to $d at "`date +%s.%N` >&2
	printf "%0.3f\n" $d
	sleep $sleeplen
done

echo "Quitting at "`date +%s.%N` >&2
echo "q"

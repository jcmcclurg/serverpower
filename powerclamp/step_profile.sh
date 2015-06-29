#!/bin/bash

sleeplen=15
steps=200
minPower=0
maxPower=50

sleep $sleeplen

for i in `seq $steps`; do
	d=`echo "scale=3; ($maxPower - $minPower)*($i - 1)/$steps + $minPower" | bc`
	echo "Step to $d at "`date +%s.%N` >&2
	echo "$d"
	sleep $sleeplen

	d=`echo "scale=3; ($maxPower - $minPower)*($steps - $i)/$steps + $minPower" | bc`
	echo "Step to $d at "`date +%s.%N` >&2
	echo "$d"
	sleep $sleeplen
done

echo "Quitting at "`date +%s.%N` >&2
echo "q"

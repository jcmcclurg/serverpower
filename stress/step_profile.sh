#!/bin/bash

sleeplen=15
steps=200
sleep $sleeplen

for i in `seq $steps`; do
	printf -v d "0.%04d" $(( (($i - 1)*10000) / $steps ))
	echo "Step to $d at "`date +%s.%N` >&2
	echo $d
	sleep $sleeplen

	printf -v d "0.%04d" $(( (($steps - $i)*10000) / $steps ))
	echo "Step to $d at "`date +%s.%N` >&2
	echo $d
	sleep $sleeplen
done

echo "Quitting at "`date +%s.%N` >&2
echo "q"

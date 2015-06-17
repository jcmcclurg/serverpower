#!/bin/bash

epids=${1:-excludedPIDs}

echo $$ >> $epids
echo $PPID >> $epids
echo $BASHPID >> $epids

sleeplen=15
steps=200
sleep $sleeplen

for i in `seq $steps`; do
	printf -v d "0.%03d" $(( (($i - 1)*1000) / $steps ))
	echo "Step to $d at "`date +%s.%N` >&2
	echo $d
	sleep $sleeplen

	printf -v d "0.%03d" $(( (($steps - $i)*1000) / $steps ))
	echo "Step to $d at "`date +%s.%N` >&2
	echo $d
	sleep $sleeplen
done

echo "Quitting at "`date +%s.%N` >&2
echo "q"

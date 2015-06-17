#!/bin/bash

epids=${1:-excludedPIDs}

echo $$ >> $epids
echo $PPID >> $epids
echo $BASHPID >> $epids

steps=120
sleep 5

for i in `seq $steps`; do
	printf -v d "0.%03d" $(( (($i - 1)*1000) / $steps ))
	echo "Step to $d at "`date +%s.%N` >&2
	echo $d
	sleep 5

	printf -v d "0.%03d" $(( (($steps - $i)*1000) / $steps ))
	echo "Step to $d at "`date +%s.%N` >&2
	echo $d
	sleep 5
done

echo "Quitting at "`date +%s.%N` >&2
echo "q"

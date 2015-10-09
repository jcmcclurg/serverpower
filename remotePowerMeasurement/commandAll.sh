#!/bin/bash

echo "Checking connectivity"
sum=1
servers="jjpowerserver1 jjpowerserver2 jjpowerserver3 jjpowerserver4 "
tmpfile="/tmp/commandAll.tmp"
sleeplen=15

until [ $sum == 0 ]; do
	rm $tmpfile > /dev/null

	echo "Pinging $servers in parallel..."
	pids=""
	for i in $servers; do
		( ping -c 1 $i > /dev/null; echo $? >> $tmpfile ) &
		pids="$pids$! "
	done
	
	cont=1
	#echo "Waiting for PIDs $pids to close..."
	while [ $cont == 1 ]; do
		cont=0
		for i in $pids; do
			if [ -e /proc/$i ]; then
				cont=1
				sleep 1
				break
			fi
		done
	done
	#echo "Finished ping."
	
	sum=`paste -s -d+ $tmpfile | bc`
	if [ $sum != 0 ]; then
		echo "The pings returned "`cat $tmpfile`
		echo "Sleeping for $sleeplen seconds..."
		sleep $sleeplen
	fi
done

for i in `seq 4`; do
	ssh jjpowerserver$i $@
done

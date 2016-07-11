#!/bin/bash

dir=$(dirname $0)

stress -c 12 & stressPID=$!


pids=$($dir/../../utilities/getChildPIDs/getChildPIDs.sh $stressPID)
echo "Started PIDS $pids"

kill -SIGSTOP $pids
echo "Stopped them."

toggle=1
while read -r; do
	cmd=$REPLY
	if [ "$cmd" == "t" ]; then
		if [ "$toggle" -eq "1" ]; then
			kill -SIGCONT $pids
			echo "Continue"
			toggle=0
		else
			kill -SIGSTOP $pids
			echo "Stop"
			toggle=1
		fi
	else
		echo "Done"
		break
	fi
done < <(python -u $dir/../../remotePowerMeasurement/measurementServer/multicast_listen.py)

echo "Killing pids"
kill -SIGKILL $pids

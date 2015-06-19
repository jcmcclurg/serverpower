#!/bin/bash

echo Welcome to the hypervisor cpu limiter. >&2
echo Type the limiting values. Press CTRL+C or q to exit. >&2
echo The range is 0-400. See xl sched-credit -c for details >&2

dom=${1:-Domain-0}
rangeMax=400
rangeMin=1

limitVal=400

trap "sudo xl sched-credit -d $dom -c $rangeMax; echo 'Reset credits to $rangeMax on exit.' >&2; exit;" SIGINT SIGTERM

while [[ "$limitVal" != "q" ]]; do
	if [ $limitVal -lt $rangeMin ]; then
		limitVal=$rangeMin
	elif [ $limitVal -gt $rangeMax ]; then
		limitVal=$rangeMax
	fi
	sudo xl sched-credit -d $dom -c $limitVal
	echo "Set credits to $limitVal" >&2
	read limitVal
done
echo "Done" >&2

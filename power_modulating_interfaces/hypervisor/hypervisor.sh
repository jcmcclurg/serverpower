#!/bin/bash

dom=${1:-ubud1}
rangeMax=1200
rangeMin=1

echo Welcome to the hypervisor cpu limiter. >&2
echo Type the limiting values. Press CTRL+C or q to exit. >&2
echo The range is $rangeMin-$rangeMax See xl sched-credit -c for details >&2

val=1200
echo "Limiting $dom." >&2

trap "sudo xl sched-credit -d $dom -c 0; echo 'Reset credits to 0 on exit.' >&2; exit;" SIGINT SIGTERM

while [[ "$val" != "q" ]]; do
	val=$( printf '%.0f' $val)
	if [ $val -lt $rangeMin ]; then
		val=$rangeMin
	elif [ $val -gt $rangeMax ]; then
		val=$rangeMax
	fi
	sudo xl sched-credit -d $dom -c $val
	read val
	#echo "Set credits to $val" >&2
done
sudo xl sched-credit -d $dom -c 0
echo "Done" >&2

#!/bin/bash

defDevice=/sys/fs/cgroup/cpu/jgroup
device=${1:-$defDevice}
group=$(basename $device)

rangeMax=1200000
rangeMin=12000
baseVal=100000
if [ ! -f $device/cpu.cfs_quota_us ]; then
	echo "Having to create the cgroup..." >&2
	sudo cgcreate -a $USER -g cpu:$group
	echo $baseVal > $device/cpu.cfs_period_us
fi

echo "Welcome to the cgroup cpu limiter. Please make sure you have" >&2
echo "the module loaded. Press CTRL+C or q to exit." >&2
echo "The range is $rangeMin to $rangeMax." >&2

val=0

trap "echo '-1' > $device/cpu.cfs_quota_us; echo 'Reset cgroup on exit.' >&2; exit;" SIGINT SIGTERM

while [[ "x$val" != "xq" ]]; do
	if [ $(echo "$val < $rangeMin" | bc) == 1 ]; then
		val=$rangeMin
		echo "Capped at $val" >&2
	elif [ $(echo "$val > $rangeMax" | bc) == 1 ]; then
		val=$rangeMax
		echo "Capped at $val" >&2
	fi
	echo $val > $device/cpu.cfs_quota_us
	cs=$(cat $device/cpu.cfs_quota_us)
	echo "Set to $cs." >&2
	read val
done

echo "-1" > $device/cpu.cfs_quota_us;
echo 'Reset cgroup' >&2;
echo "Done" >&2

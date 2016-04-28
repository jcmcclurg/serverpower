#!/bin/bash

dir=$(dirname $0)
echo "Adding $@ and its children." >&2
$dir/../../utilities/getChildPIDs/getChildPIDs.sh $@ | while read line; do
	echo "   adding $line" >&2
	echo "$line" > /sys/fs/cgroup/cpu/jgroup/cgroup.procs
done

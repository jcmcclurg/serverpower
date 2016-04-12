#!/bin/bash

echo "Adding $@ and its children." >&2
../../utilities/getChildPIDs/getChildPIDs.sh $@ | while read line; do
	echo "   adding $line" >&2
	echo "$line" > /sys/fs/cgroup/cpu/jgroup/cgroup.procs
done

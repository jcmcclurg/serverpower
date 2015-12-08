#!/bin/bash

p=${1:-$$}
lastP=$p
#pstree -l -p -s $p

pidStr=$(pstree -l -A -p -s $p | sed 's/[^0-9|]\+/ /g')
# Loop through the PIDs in reverse order.
if [[ $pidStr != *"|"* ]]; then
	pids=( $pidStr ) 
	for ((i=${#pids[@]}-1; i>=0; i--)); do
		p=${pids[$i]}
		if [ -a /proc/$p ]; then
			#echo "pstree -l -p -s $p:"
			#pstree -l -p -s $p

			pidStr=$(pstree -l -A -p -s $p | sed 's/[^0-9|]\+/ /g')
			if [[ $pidStr == *"|"* ]]; then
				echo $lastP
				break
			fi
		fi
		lastP=$p
	done
fi

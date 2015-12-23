#!/bin/bash

dir=$(dirname $0)
p=0.01
s=$dir/../stress/cstress

playbackCmd="$dir/../../utilities/playback/playback"
rapl="$dir/power_gadget"

rangeMin=21
rangeMax=36
defRampFile="$dir/interleavedRamp_${rangeMin}_${rangeMax}.csv"
dummyRampFile="$dir/interleavedRamp_${rangeMin}_${rangeMax}_short.csv"

rampFile=${1-$defRampFile}

if [[ "x$rampFile" == "xnone" ]]; then
	rampFile=$defRampFile
elif [[ "x$rampFile" == "xdummy" ]]; then
	rampFile=$dummyRampFile
else
	rampFile="$dir/${rampFile}_${rangeMin}_${rangeMax}.csv"
fi

numCPUs=12

for i in $(seq 1 $numCPUs); do
	$s -i "$i:" -d 1 -p 10 2> "/tmp/rapl_worker_${i}.log" & pids[$[ $i - 1 ]]=$!
done

echo "RangeMax = $rangeMax"
(($playbackCmd -p p -v -f $rampFile; echo "q"; echo "q"; ) | sudo $rapl ) 2>&1

echo ${pids[*]}

for pid in ${pids[*]}; do
	echo "Killing $pid" >&2
	pkill --signal INT -P $pid
	kill -s INT $pid
done

wait

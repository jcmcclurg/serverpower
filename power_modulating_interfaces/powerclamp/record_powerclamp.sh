#!/bin/bash

dir=$(dirname $0)
p=0.01
s=$dir/../stress/cstress

playbackCmd="$dir/../../utilities/playback/playback"
powerClamp="$dir/powerclamp.sh"

rangeMin=0
rangeMax=50
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
	$s -i "$i:" -d 1 -p 10 2> "/tmp/powerclamp_worker_${i}.log" & pids[$[ $i - 1 ]]=$!
done

# -w: period of 10 ms
# -d: initial duty of 0.5
# -u: do not update PID list
# -U: except if there is an error sending signals to the PIDs
# -o: do not try to find or control any children of the specified PIDs
# -p: control the specified PIDs
(($playbackCmd -v -f $rampFile; echo "q"; echo "q"; ) | sudo $powerClamp ) 2>&1

echo ${pids[*]}

for pid in ${pids[*]}; do
	echo "Killing $pid" >&2
	pkill --signal INT -P $pid
	kill -s INT $pid
done

wait

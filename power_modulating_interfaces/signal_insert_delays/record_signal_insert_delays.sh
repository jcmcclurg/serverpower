#!/bin/bash

dir=$(dirname $0)
p=0.01
s=$dir/../stress/cstress

playbackCmd="$dir/../../utilities/playback/playback"
insertDelays="$dir/signal_insert_delays"
defRampFile="$dir/../../utilities/playback/ramp_0_1.csv"
dummyRampFile="$dir/../../utilities/playback/ramp_0_1_short.csv"

rampFile=${1-$defRampFile}

if [[ "x$rampFile" == "xdummy" ]]; then
	rampFile=$dummyRampFile
fi

numCPUs=12

for i in $(seq 1 $numCPUs); do
	$s -v -i "$i:" -d 1 -p 10 2> "/tmp/signal_insert_delays_worker_${i}.log" & pids[$[ $i - 1 ]]=$!
done

# -w: period of 10 ms
# -d: initial duty of 0.5
# -u: do not update PID list
# -U: except if there is an error sending signals to the PIDs
# -o: do not try to find or control any children of the specified PIDs
# -p: control the specified PIDs
($playbackCmd -f $rampFile; echo "q"; echo "q"; ) | $insertDelays -w 0.01 -d 0.5 -u 0 -U -o -p ${pids[*]}

echo ${pids[*]}

for pid in ${pids[*]}; do
	echo "Killing $pid"
	pkill --signal INT -P $pid
	kill -s INT $pid
done

wait

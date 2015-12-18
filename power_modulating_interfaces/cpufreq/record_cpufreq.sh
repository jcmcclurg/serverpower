#!/bin/bash

dir=$(dirname $0)
p=0.01
s=$dir/../stress/cstress

playbackCmd="$dir/../../utilities/playback/playback"
cpufreq="$dir/cpufreq.sh"

read rangeMin rangeMax <<<$(sudo cpufreq-info -l)

defRampFile="$dir/../../utilities/playback/ramp_${rangeMin}_${rangeMax}.csv"
dummyRampFile="$dir/../../utilities/playback/ramp_${rangeMin}_${rangeMax}_short.csv"

rampFile=${1-$defRampFile}

if [[ "x$rampFile" == "xdummy" ]]; then
	rampFile=$dummyRampFile
fi

numCPUs=12

for i in $(seq 1 $numCPUs); do
	$s -i "$i:" -d 1 -p 10 2> "/tmp/cpufreq_worker_${i}.log" & pids[$[ $i - 1 ]]=$!
done

echo "RangeMin = $rangeMin"
echo "RangeMax = $rangeMax"
(($playbackCmd -v -f $rampFile; echo "q"; echo "q"; ) | sudo $cpufreq ) 2>&1

echo ${pids[*]}

for pid in ${pids[*]}; do
	echo "Killing $pid" >&2
	pkill --signal INT -P $pid
	kill -s INT $pid
done

wait

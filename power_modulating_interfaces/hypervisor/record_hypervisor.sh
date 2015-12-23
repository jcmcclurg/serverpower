#!/bin/bash

dir=$(dirname $0)
p=0.01
s=/home/josiah/cstress

playbackCmd="$dir/../../utilities/playback/playback"
hypervisor="$dir/hypervisor.sh"
rangeMin=1
rangeMax=1200
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

if ! sudo xl list ubud1 >/dev/null 2>/dev/null; then
	sudo xl create /etc/xen/ubud1.cfg
	sleep 15
fi

ssh $(hostname)v "/bin/bash -c '( for i in \$(seq 1 $numCPUs); do $s -i \"\$i:\" -d 1 -p 10 2> \"/tmp/hypervisor_worker\${i}.log\" & echo \$! > /tmp/hypervisor_worker\${i}.pid; done ) > /tmp/output.out'" & pid=$!

(($playbackCmd -v -f $rampFile; echo "q"; echo "q"; ) | sudo $hypervisor ) 2>&1

ssh $(hostname)v "/bin/bash -c 'killall --signal INT cstress'"
wait
ssh $(hostname)v "/bin/bash -c 'cat /tmp/output.out'"

#echo "Killing $pid" >&2
#pkill --signal INT -P $pid


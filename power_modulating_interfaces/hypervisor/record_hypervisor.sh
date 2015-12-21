#!/bin/bash

dir=$(dirname $0)
p=0.01
s=/home/josiah/cstress

playbackCmd="$dir/../../utilities/playback/playback"
hypervisor="$dir/hypervisor.sh"
defRampFile="$dir/../../utilities/playback/ramp_0_1200.csv"
dummyRampFile="$dir/../../utilities/playback/ramp_0_1200_short.csv"

rampFile=${1-$defRampFile}

if [[ "x$rampFile" == "xdummy" ]]; then
	rampFile=$dummyRampFile
fi

numCPUs=12

sudo xl create /etc/xen/ubud1.cfg
sleep 5
ssh $(hostname)v "/bin/bash -c 'for i in \$(seq 1 $numCPUs); do $s -i \"\$i:\" -d 1 -p 10 2> \"/tmp/hypervisor_worker\${i}.log\" & echo \$! > /tmp/hypervisor_worker\${i}.pid; done'" & pid=$!

# -w: period of 10 ms
# -d: initial duty of 0.5
# -u: do not update PID list
# -U: except if there is an error sending signals to the PIDs
# -o: do not try to find or control any children of the specified PIDs
# -p: control the specified PIDs
(($playbackCmd -v -f $rampFile; echo "q"; echo "q"; ) | sudo $hypervisor ) 2>&1

ssh $(hostname)v "/bin/bash -c 'sudo poweroff'"

#echo "Killing $pid" >&2
#pkill --signal INT -P $pid

wait

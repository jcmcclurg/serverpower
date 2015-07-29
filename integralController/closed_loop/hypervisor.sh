#!/bin/bash

addr="joe@"$(cat ~/ubud1_ip)

me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation.txt
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)

echo "Writing to $me.$d.output and $me.$d.power" >&2
echo "Using $powerProfile power profile." | tee -a $me.$d.output >&2

sudo xl sched-credit -d ubud1 -c 100

echo "Starting power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo $powerGadgetCmd -e 500 -c tp > $dir/$me.$d.power 2>> $me.$d.output &
pgid=$!
sleep 1
echo "Checking if power gadget $pgid exists..."
if [ ! -e /proc/$pgid ]; then
	echo "NOPE."
	exit -1
fi
echo "OK."

echo "Sarting closed loop control at "$(date +%s.%N) | tee -a $me.$d.output >&2
#(sudo $powerGadgetCmd -e 150 & pgid2=$!; $dir/playback -f $powerProfile -p s & playbackid=$!; ssh $addr "sleep 2; stress -c 4 -t 10"; scp $addr:$me.$d.output $me.$d.remote; echo "killing $pgid2 $playbackid" >&2; echo "q"; sudo kill -s SIGINT $pgid2 $playbackid) | ($dir/../integralController -s 34 -n 1 -x 400 -t 40 -k 0 -d 0 -u 10; echo "q") | $dir/../../hypervisor/hypervisor.sh >> $me.$d.output 2>> $me.$d.output
(sudo $powerGadgetCmd -e 150 & pgid2=$!; $dir/playback -f $powerProfile -p s & playbackid=$!; ssh $addr "/home/joe/research/HiBench/bin/run-all.sh >> $me.$d.output 2>> $me.$d.output"; scp $addr:$me.$d.output $me.$d.remote; echo "q"; sudo kill -s SIGINT $pgid2 $playbackid) | ($dir/../integralController -s 34 -n 1 -x 400 -t 40 -k 0 -d 0 -u 10; echo "q") | $dir/../../hypervisor/hypervisor.sh >> $me.$d.output 2>> $me.$d.output
#(sudo $powerGadgetCmd -e 150 & pgid2=$!; tee; sudo kill -s SIGINT $pgid2) | ($dir/../integralController -s 34 -n 1 -x 400 -t 40 -k 0 -d 0 -u 10; echo "q") | $dir/../../hypervisor/hypervisor.sh >> $me.$d.output 2>> $me.$d.output

ssh $addr "head -n 1 /home/joe/research/HiBench/report/hibench.report" | tee -a $me.$d.output >&2
ssh $addr "tail -n 1 /home/joe/research/HiBench/report/hibench.report" | tee -a $me.$d.output >&2
echo "Stopping power gadget $pgid at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo kill -s SIGINT $pgid

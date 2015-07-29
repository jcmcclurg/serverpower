#!/bin/bash

addr="joe@"$(cat ~/ubud1_ip)

me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation.txt
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)

echo "Writing to $me.$d.output and $me.$d.power" >&2
echo "Using $powerProfile power profile." | tee -a $me.$d.output >&2
echo "Starting power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo $powerGadgetCmd -e 500 -c tp > $dir/$me.$d.power 2>> $me.$d.output &
pgid=$!
sleep 1
echo "Checking if $pgid exists..."
if [ ! -e /proc/$pgid ]; then
	echo "NOPE."
	exit -1
fi
echo "OK."

echo "Sarting closed loop control at "$(date +%s.%N) | tee -a $me.$d.output >&2
(sudo $powerGadgetCmd -e 150 & pgid2=$!; (ssh $addr "/home/joe/research/HiBench/bin/run-all.sh >> $me.$d.output 2>> $me.$d.output"; scp $addr:$me.$d.output $me.$d.remote; echo "q"; sudo kill -s SIGINT $pgid2) & sleep 5; $dir/playback -f $powerProfile -p s) | ($dir/../integralController -s 34 -n 0 -x 1 -t 0.1 -k 0.005 -d 0 -u 10; echo "q") | $dir/donothing.sh >> $me.$d.output 2>> $me.$d.output

ssh $addr "head -n 1 /home/joe/research/HiBench/report/hibench.report" | tee -a $me.$d.output >&2
ssh $addr "tail -n 1 /home/joe/research/HiBench/report/hibench.report" | tee -a $me.$d.output >&2
echo "Stopping power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo kill -s SIGINT $pgid

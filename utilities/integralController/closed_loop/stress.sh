#!/bin/bash


me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation_25_40.csv
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)
echo "Writing to $me.$d.output and $me.$d.power" >&2
echo "Taking setpoints from $powerProfile" | tee -a $me.$d.output >&2

echo "Starting power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo $powerGadgetCmd -e 500 -c tp > $dir/$me.$d.power 2>> $me.$d.output &
pgid=$!

echo "Sarting closed loop control at "$(date +%s.%N) | tee -a $me.$d.output >&2
(sudo $powerGadgetCmd -e 150 & pgid2=$!; sleep 5; $dir/playback -f $powerProfile -p s; echo "q"; sudo kill -s SIGINT $pgid2) | ($dir/../integralController -s 34 -n 0 -x 1 -t 0.1 -k 0.005 -d 0 -u 10; echo "q") | $dir/../../stress/go_four_cstress.sh >> $me.$d.output 2>> $me.$d.output

echo "Stopping power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo kill -s SIGINT $pgid

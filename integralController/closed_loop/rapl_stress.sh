#!/bin/bash

me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation_25_40.csv
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)

echo "Writing to $me.$d.output and $me.$d.power" >&2
echo "Starting power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
echo "Using $powerProfile as power profile." | tee -a $me.$d.output >&2
sudo $powerGadgetCmd -e 500 -c tp > $dir/$me.$d.power 2>> $me.$d.output &
pgid=$!

echo "Sarting closed loop control at "$(date +%s.%N) | tee -a $me.$d.output >&2
( ( ($dir/playback -f $powerProfile > /dev/null 2> /dev/null; echo "q") | $dir/../../stress/go_four_cstress.sh >> $me.$d.output 2>> $me.$d.output; echo "q") & sleep 5; $dir/playback -f $powerProfile -p p) | ( sudo $dir/../../rapl/power_gadget -e 150 -q ) >> $me.$d.output 2>> $me.$d.output

echo "Stopping power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo kill -s SIGINT $pgid

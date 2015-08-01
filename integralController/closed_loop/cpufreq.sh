#!/bin/bash

me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation.txt
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)

echo "Writing to $me.$d.output and $me.$d.power" >&2
echo "Starting power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
echo "Using $powerProfile as power profile." | tee -a $me.$d.output >&2

sudo $powerGadgetCmd -e 500 -c tp > $dir/$me.$d.power 2>> $me.$d.output &
pgid=$!

echo "Starting closed loop control at "$(date +%s.%N) | tee -a $me.$d.output >&2

read rangeMin rangeMax <<<$(sudo cpufreq-info -l)
t=$[ ($rangeMax-$rangeMin)/10 ]
echo "Range is $rangeMin to $rangeMax, with gain set to $t" | tee -a $me.$d.output >&2

# With tee and stress
(sudo $powerGadgetCmd -e 150 & pgid2=$!; stress -c 4 & playbackid=$!; tee; sudo kill -s SIGINT $pgid2; pkill -P $playbackid; echo "q") | ($dir/../integralController -s 34 -n $rangeMin -x $rangeMax -t $t -k 0 -d 0 -u 10; sleep 1; echo "q") | sudo $dir/../../cpufreq/cpufreq.sh >> $me.$d.output 2>> $me.$d.output

# With integral controller
#(sudo $powerGadgetCmd -e 150 & pgid2=$!; $dir/playback -f $powerProfile -p s & playbackid=$!; $dir/../../../HiBench/bin/run-all.sh >> $me.$d.remote 2>> $me.$d.remote; sudo kill -s SIGINT $pgid2 $playbackid; echo "q") | ($dir/../integralController -s 34 -n 1200000 -x 2500000 -t 130000 -k 0 -d 0 -u 10; sleep 1; echo "q") | sudo $dir/../../cpufreq/cpufreq.sh >> $me.$d.output 2>> $me.$d.output

head -n 1 $dir/../../../HiBench/report/hibench.report | tee -a $me.$d.output >&2
tail -n 1 $dir/../../../HiBench/report/hibench.report | tee -a $me.$d.output >&2

echo "Stopping power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo kill -s SIGINT $pgid
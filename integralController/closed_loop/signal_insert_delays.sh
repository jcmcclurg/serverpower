#!/bin/bash

me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation.txt
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)

echo "Writing to $me.$d.output and $me.$d.power" >&2
parentPID=$($dir/getHighestPPID.sh $$)
echo "Excluding $parentPID (and children) and $@" | tee -a $me.$d.output >&2

echo "Starting power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo $powerGadgetCmd -e 500 -c tp > $dir/$me.$d.power 2>> $me.$d.output &
pgid=$!

#echo "Starting workload at "$(date +%s.%N) | tee -a $me.$d.output >&2
#$dir/test.sh &
#workid=$!
#sleep 1
#pstree -p $workid
#(sudo $powerGadgetCmd -e 150 & pgid2=$!; $dir/donothing.sh & playbackid=$!; tee; sudo kill -s SIGINT $pgid2; pkill -P $playbackid; echo "q") | ($dir/../integralController -s 34 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | (j=$($dir/getChildrenPIDs.sh $workid | sed 's/ /\\|/g'); $dir/getChildrenPIDs.sh $parentPID | sed "s/\\($j\\)//g" & $dir/donothing.sh)

echo "Using $powerProfile as power profile." | tee -a $me.$d.output >&2
echo "Starting closed loop control at "$(date +%s.%N) | tee -a $me.$d.output >&2

# With tee and stress
#(sudo $powerGadgetCmd -e 150 & pgid2=$!; (sleep 1; stress -c 4) & playbackid=$!; tee; sudo kill -s SIGINT $pgid2; killall stress; echo "q") | ($dir/../integralController -s 34 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | $dir/../../signal_insert_delays/insertDelays -U -u 5 -x -e $($dir/getChildrenPIDs.sh $parentPID) "$@" >> $me.$d.output 2>> $me.$d.output

# With integral controller
#(sudo $powerGadgetCmd -e 150 & pgid2=$!; $dir/playback -f $powerProfile -p s & playbackid=$!; sleep 10; sudo kill -s SIGINT $pgid2 $playbackid; echo "q") | ($dir/../integralController -s 34 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | $dir/../../signal_insert_delays/insertDelays -U -u 5 -x -e $($dir/getChildrenPIDs.sh $parentPID) "$@" >> $me.$d.output 2>> $me.$d.output
(sudo $powerGadgetCmd -e 150 & pgid2=$!; $dir/playback -f $powerProfile -p s & playbackid=$!; (sleep 2; $dir/../../../HiBench/bin/run-all.sh) >> $me.$d.remote 2>> $me.$d.remote; sudo kill -s SIGINT $pgid2 $playbackid; echo "q") | ($dir/../integralController -s 34 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | $dir/../../signal_insert_delays/insertDelays -v -U -u 10 -w 0.3 -x -e $($dir/getChildrenPIDs.sh $parentPID) "$@" >> $me.$d.output 2>> $me.$d.output

head -n 1 $dir/../../../HiBench/report/hibench.report | tee -a $me.$d.output >&2
tail -n 1 $dir/../../../HiBench/report/hibench.report | tee -a $me.$d.output >&2

echo "Stopping power gadget at "$(date +%s.%N) | tee -a $me.$d.output >&2
sudo kill -s SIGINT $pgid

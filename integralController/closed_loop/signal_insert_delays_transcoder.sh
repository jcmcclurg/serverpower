#!/bin/bash

me=$(basename $0 .sh)
dir=$(dirname $0)
powerProfile=$dir/powerDeviation.txt
powerGadgetCmd=$dir/../../signal_insert_delays/power_gadget_whileloop/power_gadget
d=$(date +%s.%N)

avconv -i $dir/../../transcoders/videos/cut.mp4 -r 30 -y $dir/../../transcoders/videos/output.avi 2>> $me.deletme >> $me.deleteme &
workid=$!
echo "Starting workload $workid at "$(date +%s.%N)
sleep 1
pstree -p $workid
echo "Running. Type CTRL+D to exit everything immediately."
echo "Type CTRL+C to exit controller and leave workload running."
echo "Type q to exit controller, then hit ENTER to exit workload."
echo "Type s<power> to set the power in watts (example: s20)."
(sudo $powerGadgetCmd -e 150 & pgid2=$!; tee; sudo kill -s SIGINT $pgid2; pkill -P $workid; echo "q") | ($dir/../integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | $dir/../../signal_insert_delays/insertDelays -U -d 0.5 -p $workid
#$dir/../../signal_insert_delays/insertDelays -v -U -u 5 -d 0.5 -p $workid
echo "Killing $workid and children."
kill -s SIGINT $workid

sleep 1
pstree -p $workid

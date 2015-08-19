#!/bin/bash

#go_transcoder_FMD_control_test.sh
#	avconv | fmd2server (using fps & freq to output pwr_set) | integralController | insertDelays (delaying avconv)
dir="/home/powerserver/joe/serverpower"
vid_input="$dir/transcoders/videos/cut.mp4"
vid_output="$dir/transcoders/videos/output10.avi"
power_monitor="$dir/rapl/power_gadget"
transcd_ctrlr="$dir/xbeecom/RS-232-Lib/transcoderFMDController"
transcd_log="$dir/xbeecom/RS-232-Lib/test.csv"
integral_ctrlr="$dir/integralController/integralController"
insert_delays="$dir/signal_insert_delays/insertDelays"

#rm $transcd_log

(unbuffer -p avconv -i $vid_input -r 30 -y $vid_output 2>/dev/null; echo "S"; sleep 1; echo "q") | (sudo $power_monitor -e 150 & pgid2=$!; sudo $transcd_ctrlr -r 500 -p 16 -b 115200 -M 30 -m 16 -a 1 -B 15000 -o $transcd_log; sudo kill -s SIGINT $pgid2; sleep 1; echo "q") | ($integral_ctrlr -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | (sleep 1; $insert_delays -U -d 0.5 -p $(pgrep avconv))

#| ~/joe/serverpower/integralController/integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | ~/joe/serverpower/signal_insert_delays/insert 



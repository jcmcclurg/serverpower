#!/bin/bash

#go_transcoder_FMD_control_test.sh
#	avconv | fmd2server (using fps & freq to output pwr_set) | integralController | insertDelays (delaying avconv)
dir="/home/powerserver/joe/serverpower"

(avconv -i $dir/transcoders/videos/cut.mp4 -r 30 -y $dir/transcoders/videos/output.avi 2>/dev/null; echo "S"; sleep 1; echo "q") | (sudo ~/joe/serverpower/rapl/power_gadget -e 150 & pgid2=$!; sudo ~/joe/serverpower/xbeecom/RS-232-Lib/transcoderFMDController -r 500 -p 18 -b 115200 -M 28 -m 18 -a 1 -B 10000 -o ~/joe/serverpower/xbeecom/RS-232-Lib/test.csv ; sudo kill -s SIGINT $pgid2; sleep 1; echo "q") | (~/joe/serverpower/integralController/integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | (sleep 1; ~/joe/serverpower/signal_insert_delays/insertDelays -U -d 0.5 -p $(pgrep avconv))

#| ~/joe/serverpower/integralController/integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | ~/joe/serverpower/signal_insert_delays/insert 



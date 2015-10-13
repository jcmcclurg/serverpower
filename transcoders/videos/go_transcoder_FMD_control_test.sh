#!/bin/bash

#go_transcoder_FMD_control_test.sh
#	avconv | fmd2server (using fps & freq to output pwr_set) | integralController | insertDelays (delaying avconv)

 tee >(stdbuf -i0 -o0 avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output.avi 2>/dev/null & workid=$! ) | unbuffer -p tee >(~/joe/serverpower/xbeecom/RS-232-Lib/transcoderFMDController -r 500 -p 17 -b 115200 -M 44 -m 10 -a 1 -B 10000 -o ~/joe/serverpower/xbeecom/RS-232-Lib/test.csv)  #| {(sudo ~/joe/serverpower/rapl/power_gadget -e 150 & pgid2=$!; tee; sudo kill -s SIGINT $pgid2; pkill -P $workid; echo "q") | (~/joe/serverpower/integralController/integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | ~/joe/serverpower/signal_insert_delays/insertDelays -U -d 0.5 -p $workid }

#| ~/joe/serverpower/integralController/integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | ~/joe/serverpower/signal_insert_delays/insert 



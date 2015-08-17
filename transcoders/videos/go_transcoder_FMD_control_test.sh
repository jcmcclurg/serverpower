#!/bin/bash

#go_transcoder_FMD_control_test.sh
#	avconv | fmd2server (using fps & freq to output pwr_set) | integralController | insertDelays (delaying avconv)
unbuffer avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/server/transcoders/videos/output.avi & workerid=$!; tee; pkill -P $workerid; echo "S"; sleep 2; echo "q") | unbuffer ~/joe/serverpower/xbeecom/RS-232-Lib/transcoderFMDController -r 500 -p 16 -b 115200 -M 44 -m 10 -a 1 -B 10000 -o test.csv #| ~/joe/serverpower/integralController/integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10; sleep 1; echo "q") | ~/joe/serverpower/signal_insert_delays/insert 



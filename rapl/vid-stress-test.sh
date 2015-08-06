#!/bin/bash

avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output.avi > ~/joe/serverpower/transcoders/videos/log.csv & pgid=$!
avconv -i ~/joe/serverpower/transcoders/videos/cut1.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output1.avi > ~/joe/serverpower/transcoders/videos/log1.csv & pgid1=$!
avconv -i ~/joe/serverpower/transcoders/videos/cut1.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output2.avi > ~/joe/serverpower/transcoders/videos/log2.csv & pgid2=$!
avconv -i ~/joe/serverpower/transcoders/videos/cut1.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output3.avi > ~/joe/serverpower/transcoders/videos/log3.csv & pgid3=$!
avconv -i ~/joe/serverpower/transcoders/videos/cut1.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output4.avi > ~/joe/serverpower/transcoders/videos/log4.csv & pgid4=$!

tee >(stdbuf -i0 -o0 ../xbeecom/RS-232-Lib/fmd2server -r 500 -p 17 -b 115200 -M 40 -m 20 -a 1 -o ../xbeecom/RS-232-Lib/test.csv) | unbuffer -p tee >(./power_gadget -e 500)

kill $pgid
kill $pgid1
kill $pgid2
kill $pgid3
kill $pgid4

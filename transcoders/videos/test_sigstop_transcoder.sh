#!/bin/bash

#avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output.avi > ~/joe/serverpower/transcoders/videos/log.csv & pgid=$!

avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output.avi & pgid=$!

echo $pgid
#~/joe/serverpower/signal_insert_delays/insertDelays -v -U -u 10 -w 0.3 -o -p $pgid

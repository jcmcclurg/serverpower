#!/bin/bash

#avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output.avi > ~/joe/serverpower/transcoders/videos/log.csv & pgid=$!

avconv -i ~/joe/serverpower/transcoders/videos/cut.mp4 -r 30 -y ~/joe/serverpower/transcoders/videos/output.avi & pid=$!
#stress -m 2 & pid2=$!

~/joe/serverpower/signal_insert_delays/insertDelays -U -d 0.5 -w 0.01 -p $pid #$pid2

#!/bin/bash

:<<'COMMENT'
Can monitor everything by:
	
	COMMAND						|	DIRECTORY
	----------------------------|---------------------
	tail -f calcSet3Data.csv	|	xbeecom/RS-232-Lib

Can kill everything by:
	
	./killPIDs.sh

COMMENT

maxPower=$1
minPower=$2
if [[ -n "$maxPower" ]]; then
	:
else
	maxPower=34
fi
if [[ -n "$minPower" ]]; then
	:
else
	minPower=18
fi

dir="/home/powerserver/joe/serverpower"
vIn="$dir/transcoders/videos"
vOut="$dir/transcoders/videos"
power_gadget="$dir/rapl/power_gadget"
calcSetpoint="$dir/xbeecom/RS-232-Lib/calcSetpoint"
logPath="$dir/xbeecom/RS-232-Lib"
integralController="$dir/integralController/integralController"
insertDelays="$dir/signal_insert_delays/insertDelays"
getFreq="$dir/xbeecom/RS-232-Lib/getFreq"

while true; do
	avconv -i $vIn/cut4.mp4 -r 30 -y $vOut/out4.avi > frames3 & pid=$!
	#setpoint = $(echo "setpoint$pid")
	(tail -f -q --pid=$pid freq frames3; echo "q") | $calcSetpoint -d 0 -M $maxPower -m $minPower -o $logPath/calcSet3Data.csv > setpoint3 &
	(tail -f -q --pid=$pid setpoint3 power; echo "q") | $integralController -s $minPower -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $pid &
	wait $pid
	#rm $setpoint
done

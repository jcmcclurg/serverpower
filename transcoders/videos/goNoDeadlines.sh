#!/bin/bash

:<<'COMMENT'
Can monitor everything by:
	
	COMMAND						|	DIRECTORY
	----------------------------|---------------------
	tail -f calcSet1Data.csv	|	xbeecom/RS-232-Lib
	tail -f calcSet2Data.csv	|	xbeecom/RS-232-Lib
	tail -f data.csv			|	rapl/data

Can kill everything by:
	
	./killPIDs.sh

COMMENT

dir="/home/powerserver/joe/serverpower"
vIn="$dir/transcoders/videos/cut.mp4"
vOut="$dir/transcoders/videos/output10.avi"
power_gadget="$dir/rapl/power_gadget"
calcSetpoint="$dir/xbeecom/RS-232-Lib/calcSetpoint"
logPath="$dir/xbeecom/RS-232-Lib"
integralController="$dir/integralController/integralController"
insertDelays="$dir/signal_insert_delays/insertDelays"
getFreq="$dir/xbeecom/RS-232-Lib/getFreq"

while true; do
	avconv -i $vIn -r 30 -y $vOut > frames3 & pid=$!
	#setpoint = $(echo "setpoint$pid")
	(tail -f -q --pid=$pid freq frames3; echo "q") | $calcSetpoint -d 0 -M 34 -m 18 -o $logPath/calcSet3Data.csv> setpoint3 &
	(tail -f -q --pid=$pid setpoint3 power; echo "q") | $integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $pid &
	echo $pid
	wait $pid
	#rm $setpoint
done

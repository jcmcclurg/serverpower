#!/bin/bash

#!/bin/bash
# two transcoder workloads with throughput constraints
# freq output
# with named pipes

:<<'COMMENT'
Can monitor everything by:
	
	COMMAND						|	DIRECTORY
	----------------------------|---------------------
	tail -f calcSet1Data.csv	|	xbeecom/RS-232-Lib
	tail -f calcSet2Data.csv	|	xbeecom/RS-232-Lib
	tail -f powerLog.log		|	remotePowerMeasurement/powerLog.log

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

echo $maxPower
echo $minPower

dir="/home/powerserver/joe/serverpower"
vIn="$dir/transcoders/videos"
vOut="$dir/transcoders/videos/"
powerMeasure="$dir/remotePowerMeasurement/remotePowerMeasurement.sh"
powerGadget="$dir/rapl/power_gadget"
calcSetpoint="$dir/xbeecom/RS-232-Lib/calcSetpoint"
logPath="$dir/xbeecom/RS-232-Lib"
integralController="$dir/integralController/integralController"
insertDelays="$dir/signal_insert_delays/insertDelays"
getFreq="$dir/xbeecom/RS-232-Lib/getFreq"

echo "begin test"
sudo $getFreq > freq &
avconv -i $vIn/cut1.mp4 -r 30 -y $vOut/out1.avi > frames1 & echo $! > avconv1.pid
#avconv -i $vIn/cut2.mp4 -r 30 -y $vOut/out2.avi > frames2 & echo $! > avconv2.pid
tail -f -q freq frames1 | $calcSetpoint -d 1 -M $maxPower -m $minPower -B 3000 -o $logPath/calcSet1Data.csv > setpoint1 & 
#tail -f -q freq frames2 | $calcSetpoint -d 1 -M $maxPower -m $minPower -B 3000 -o $logPath/calcSet2Data.csv > setpoint2 & 
$powerMeasure > power &
sudo $powerGadget -e 500 > /dev/null &
tail -f -q setpoint1 power | $integralController -s 70 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $(cat avconv1.pid) &
#tail -f -q setpoint2 power | $integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $(cat avconv2.pid) &

echo "started test programs"

./goNoDeadlines.sh $maxPower $minPower &
./goNoDeadlines2.sh $maxPower $minPower &

echo "running goNoDeadlines"

:<<'COMMENT'
# DR Load without deadline:
while true; do
	avconv -i $vIn -r 30 -y $vOut > output & pid=$!
	(tail -f -q --pid=$pid freq; echo "q") | $calcSetpoint -d 0 -M 34 -m 18 > setpoint3 &
	(tail -f -q --pid=$pid setpoint3 power; echo "q") | $integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $pid &


	echo $pid
	wait $pid
done
COMMENT



#!/bin/bash
dir="/home/powerserver/joe/serverpower"
powerGadget="$dir/rapl/power_gadget"
initializePowerMeasure="$dir/remotePowerMeasurement/initializeAllStreams.sh"
powerMeasure="$dir/remotePowerMeasurement/streamPowerFast.sh"
stopPowerMeasure="$dir/remotePowerMeasurement/stopStreaming.sh"
insertDelays="$dir/signal_insert_delays/insertDelays"

mkfifo pipe
rm powerMeasure.csv pg_data.csv remoteData.csv

$initializePowerMeasure
cat < pipe | sudo $powerGadget -e 100 > /dev/null & pgid=$! 
echo "pgid=$pgid"
$powerMeasure > powerMeasure.csv & pmid=$! 
echo "pmid=$pmid"

stress -m 10 & sid=$!

COUNTER=0
iterations=10000
sleepTime=$(bc <<< "scale=3; 10/$iterations")
while [  $COUNTER -lt $(($iterations-9000)) ]; do
	bc <<< "scale=3; 0.0001+$COUNTER/$iterations"
	let COUNTER=COUNTER+1 
	sleep 1
done | $insertDelays -U -d 0.001 -w 0.00001 -p $sid
echo "loop finished"
kill -KILL $(pgrep stress)
$stopPowerMeasure
echo "q" > pipe
#sudo kill -KILL $pgid 
rm pipe

./formatData.sh

#cat < pipe | $integralController -s $minPower -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 #| $insertDelays -U -d 0.5 -p $(cat avconv1.pid) &

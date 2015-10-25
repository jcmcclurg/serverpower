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
sudo $powerGadget -e 100 > /dev/null & pgid=$! 
echo "pgid=$pgid"
$powerMeasure > powerMeasure.csv & pmid=$! 
echo "pmid=$pmid"

stress -m 10 & sid=$!

COUNTER=0
iterations=3
while [  $COUNTER -lt $iterations ]; do
	date +%T:%N > iteration$COUNTER
	bc <<< "scale=3; $COUNTER+0.001"
	let COUNTER=COUNTER+1 
	sleep 3
done | $insertDelays -U -d 0.001 -w 0.001 -p $sid
echo "loop finished"
kill -KILL $(pgrep stress)
$stopPowerMeasure
sudo kill -KILL $(pgrep power)
sudo kill -KILL $(pgrep Freq)
sudo kill -KILL $(pgrep integral)
sudo kill -KILL $(pgrep insert)
sudo kill -KILL $(pgrep calc)


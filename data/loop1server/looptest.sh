#!/bin/bash
dir="/home/powerserver/joe/serverpower"
powerGadget="$dir/rapl/power_gadget"
initializePowerMeasure="$dir/remotePowerMeasurement/initializeStreaming.sh"
powerMeasure="$dir/remotePowerMeasurement/streamPowerFast.sh"
stopPowerMeasure="$dir/remotePowerMeasurement/stopStreaming.sh"
insertDelays="$dir/signal_insert_delays/insertDelays"
getFreq="$dir/remotePowerMeasurement/streamFrequency.sh"
integralController="$dir/integralController/integralController"
calcSetpoint="$dir/xbeecom/RS-232-Lib/calcSetpoint"

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
echo "maxPower = $maxPower"
echo "minPower = $minPower"

#mkfifo pipe 
mkfifo pipe2
rm powerMeasure.csv pg_data.csv remoteData.csv calcSet1Data.csv

#cat < pipe | $powerGadget -e 150 > pipe2 & pgid=$! 

$initializePowerMeasure
$powerMeasure | tee powerMeasure.csv | unbuffer -p cut -d , -f 5 > pipe2 & pmid=$! 

$getFreq | $calcSetpoint -d 0 -M $maxPower -m $minPower -o calcSet1Data.csv > pipe2 & 

stress -m 10 & sid=$!

cat < pipe2 | $integralController -s $maxPower -n 0.001 -x 0.999 -t 0.3 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -w 0.001 -p $sid &

echo "entering sleep"
sleep 600
echo "test finished"
kill -KILL $(pgrep stress)

$stopPowerMeasure
#echo "q" > pipe
rm pipe pipe2
sudo kill -KILL $(pgrep power)
sudo kill -KILL $(pgrep Freq)
sudo kill -KILL $(pgrep integral)
sudo kill -KILL $(pgrep insert)
sudo kill -KILL $(pgrep calc)

./formatData.sh


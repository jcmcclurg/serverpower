#!/bin/bash
dir="/home/powerserver/joe/serverpower"
powerGadget="$dir/rapl/power_gadget"
initializePowerMeasure="$dir/remotePowerMeasurement/initializeStreaming.sh"
powerMeasure="$dir/remotePowerMeasurement/streamPowerFast.sh"
stopPowerMeasure="$dir/remotePowerMeasurement/stopStreaming.sh"
insertDelays="$dir/signal_insert_delays/insertDelays"

mkfifo pipe

$initializePowerMeasure
cat < pipe | sudo $powerGadget -e 100 > /dev/null & pgid=$! 
echo "pgid=$pgid"
$powerMeasure > powerMeasure.csv & pmid=$! 
echo "pmid=$pmid"

stress -c 1 & sid=$!

COUNTER=0
iterations=1000
sleepTime=$(bc <<< "scale=3; 10/$iterations")
while [  $COUNTER -lt $iterations ]; do
	bc <<< "scale=3; $COUNTER/$iterations"
	let COUNTER=COUNTER+1 
	sleep 0.1
done | $insertDelays -U -d 0.5 -w 0.0001 -p $sid
echo "loop finished"
kill -KILL $(pgrep stress)
$stopPowerMeasure
echo "q" > pipe
#sudo kill -KILL $pgid 
rm pipe

# get power_gadget output data
awk -F [::.,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' $dir/rapl/data/data.csv > time_pg.csv #> /dev/null
cut -d, -f3 $dir/rapl/data/data.csv > pkg_pg.csv #> /dev/null
awk -F [,] '{printf("%.5G\n",$5)}' $dir/rapl/data/data.csv > dram_pg.csv
paste -d ',' time_pg.csv pkg_pg.csv dram_pg.csv > pg_data.csv # final output file
rm time_pg.csv pkg_pg.csv dram_pg.csv # clean up

# get powerMeasurement output data
awk -F [' '] '{printf("%.10f,%.3f,\n",$2,$5)}' powerMeasure.csv > powerLogDecimated.csv
awk -F [,] '{system("date -d @"$1" +%T:%N")}' powerLogDecimated.csv > powerFormated.csv
awk -F [:::,] '{printf("%.16G\n",($1*3600+$2*60+$3+$4/1000000000))}' powerFormated.csv > timeRemote.csv #> /dev/null
cut -d, -f2 powerLogDecimated.csv > powerRemote.csv
paste -d ',' timeRemote.csv powerRemote.csv > remoteData.csv # final output file
rm powerLogDecimated.csv powerFormated.csv timeRemote.csv powerRemote.csv  # clean up


#cat < pipe | $integralController -s $minPower -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 #| $insertDelays -U -d 0.5 -p $(cat avconv1.pid) &

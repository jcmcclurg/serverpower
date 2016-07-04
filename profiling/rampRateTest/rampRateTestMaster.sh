#!/bin/bash

dir=$( realpath $( dirname $0 ) )

ip="192.168.1.200"
logfile=$(cygpath -w "$dir/powerlog3.log")

echo "Cleaning up old power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$logfile" --quiet

echo "Starting power stream..." >&2
wget -O - "http://$ip:8282/stream?command=start&length=10000&blockLen&type=csvScaled&fields=01234567&address=$logfile" --quiet

remotecmd="/bin/bash -c '/home/josiah/serverpower/profiling/rampRateTest/rampRateTestClient.sh'"

ssh josiah@192.168.1.101 "$remotecmd" & testPID[0]=$!
ssh josiah@192.168.1.102 "$remotecmd" & testPID[1]=$!
ssh josiah@192.168.1.103 "$remotecmd" & testPID[2]=$!
ssh josiah@192.168.1.104 "$remotecmd" & testPID[3]=$!

echo "Test PIDs are ${testPID[*]}. Waiting until they finish." >&2

$dir/../../remotePowerMeasurement/measurementServer/multicast_send.py < <(sleep 1; echo 't'; sleep 3; echo 't'; sleep 3; echo 't'; sleep '3'; echo 't'; sleep 3; echo 's')

wait ${testPID[0]}; echo "Test 1 finished."
wait ${testPID[1]}; echo "Test 2 finished."
wait ${testPID[2]}; echo "Test 3 finished."
wait ${testPID[3]}; echo "Test 4 finished."

echo "Cleaning up power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$logfile&type=csvScaled" --quiet

echo ""

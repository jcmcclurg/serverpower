#!/bin/bash

dir=$( realpath $( dirname $0 ) )
multicastListenCmd="$dir/../remotePowerMeasurement/measurementServer/multicast_listen.py"

ip="192.168.1.200"
port=9999
group="224.1.1.1"

exp=${1-none}
if [[ "x$exp" == "xnone" ]]; then
	ls $dir/experiments
	echo -n "Choose an experiment: " >&2
	read exp
fi

#date=$( date +%s.%N )
date="today"

echo "Running $exp on $date..."
expdir="$dir/experiments/$exp/$date"
echo "Creating run directory $expdir"
mkdir $expdir

echo "Cleaning up old power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$group&port=$port" --quiet

echo ""
echo "Starting power logger $multicastListenCmd ($expdir/powerlog.log)..." >&2
/usr/bin/python -u "$multicastListenCmd" -l "$expdir/powerlog.log" -n -a $group -p $port -s 4096 2> "$expdir/errs.log" & loggerPID=$!

echo "Logger PID=$loggerPID. Starting power stream..." >&2
wget -O - "http://$ip:8282/stream?command=start&length=10000&blockLength=500&type=power&fields=1234567&delimiter=%20&address=$group&port=$port" --quiet

remotecmd="/bin/bash -c 'date +%s.%N > /tmp/${date}.perflog && echo \" \" >> /tmp/${date}.perflog && sudo perf stat -e cpu-cycles -e instructions -a -I 100 -x \  -o /tmp/${date}.perflog --append'"
ssh josiah@192.168.1.101 "$remotecmd" & perfPID[0]=$!
ssh josiah@192.168.1.102 "$remotecmd" & perfPID[1]=$!
ssh josiah@192.168.1.103 "$remotecmd" & perfPID[2]=$!
ssh josiah@192.168.1.104 "$remotecmd" & perfPID[3]=$!

echo "Perf PIDs are ${perfPID[*]}. Starting test run..." >&2
remotecmd="/bin/bash -c 'date +%s.%N > /tmp/${date}.testlog && echo \" \" >> /tmp/${date}.testlog && /home/josiah/research/serverpower/$exp/record_$exp.sh > /tmp/${date}.testlog'"
ssh josiah@192.168.1.101 "$remotecmd" & testPID[0]=$!
sleep 1;
ssh josiah@192.168.1.102 "$remotecmd" & testPID[1]=$!
sleep 1;
ssh josiah@192.168.1.103 "$remotecmd" & testPID[2]=$!
sleep 1;
ssh josiah@192.168.1.104 "$remotecmd" & testPID[3]=$!

echo "Test PIDs are ${testPID[*]}. Waiting until they finish." >&2
wait ${testPID[0]}; echo "Test 1 finished. Stopping perf."; /bin/kill -f -s INT ${perfPID[0]}
wait ${testPID[1]}; echo "Test 2 finished. Stopping perf."; /bin/kill -f -s INT ${perfPID[1]}
wait ${testPID[2]}; echo "Test 3 finished. Stopping perf."; /bin/kill -f -s INT ${perfPID[2]}
wait ${testPID[3]}; echo "Test 4 finished. Stopping perf."; /bin/kill -f -s INT ${perfPID[3]}

echo "Getting perf files..." >&2
remotecmd="/bin/bash -c ''"
scp josiah@192.168.1.101:/tmp/$date.perflog $expdir/1.perflog
scp josiah@192.168.1.101:/tmp/$date.testlog $expdir/1.testlog
scp josiah@192.168.1.102:/tmp/$date.perflog $expdir/2.perflog
scp josiah@192.168.1.102:/tmp/$date.testlog $expdir/2.testlog
scp josiah@192.168.1.103:/tmp/$date.perflog $expdir/3.perflog
scp josiah@192.168.1.103:/tmp/$date.testlog $expdir/3.testlog
scp josiah@192.168.1.104:/tmp/$date.perflog $expdir/4.perflog
scp josiah@192.168.1.104:/tmp/$date.testlog $expdir/4.testlog

echo "Cleaning up power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$group&port=$port" --quiet

echo ""
echo "Closing power logger (PID=$loggerPID)..." >&2
/bin/kill -f -s INT $loggerPID

#!/bin/bash

dir=$( realpath $( dirname $0 ) )
multicastListenPath="$dir/../remotePowerMeasurement/measurementServer/multicast_listen.py"

ip="192.168.1.200"
port=9999
group="224.1.1.1"

exp=${1-none}
expArg=${2-none}

if [[ "x$exp" == "xnone" ]]; then
	ls $dir/experiments
	echo -n "Choose an experiment: " >&2
	read exp
fi
powerGadgetPath="/home/josiah/research/serverpower/utilities/power_gadget_whileloop/power_gadget"

testPath="/home/josiah/research/serverpower/power_modulating_interfaces/$exp/record_$exp.sh"
date=$( date +%s.%N )
if [[ "x$expArg" == "xnone" ]]; then
	echo "Using default parameters." >&2
elif [[ "x$expArg" == "xdummy" ]]; then
	testPath="$testPath dummy"
	date="today"
else
	testPath="$testPath $expArg"
fi


echo "Running $exp on $date..."
expdir="$dir/experiments/$exp/$date"
echo "Creating run directory $expdir"
mkdir $expdir

echo "Cleaning up old power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$group&port=$port" --quiet

echo ""
echo "Starting power logger $multicastListenPath ($expdir/powerlog.log)..." >&2
/usr/bin/python -u "$multicastListenPath" -l "$expdir/powerlog.log" -n -a $group -p $port -s 4096 2> "$expdir/errs.log" & loggerPID=$!

echo "Logger PID=$loggerPID. Starting power stream..." >&2
wget -O - "http://$ip:8282/stream?command=start&length=10000&blockLength=500&type=power&fields=1234567&delimiter=%20&address=$group&port=$port" --quiet

perfCmd="date +%s.%N > /tmp/${date}.perflog && echo \" \" >> /tmp/${date}.perflog && sudo perf stat -e cpu-cycles -e instructions -a -I 100 -x \\  -o /tmp/${date}.perflog --append"
powerGadgetCmd="date +%s.%N > /tmp/${date}.pgadglog && echo \" \" >> /tmp/${date}.pgadglog && sudo $powerGadgetPath -e 100 -c tp >> /tmp/${date}.pgadglog"
testCmd="date +%s.%N > /tmp/${date}.testlog && echo \" \" >> /tmp/${date}.testlog && $testPath >> /tmp/${date}.testlog"

remotecmd="/bin/bash -c '$perfCmd & pfpid=\$!; $powerGadgetCmd & pgpid=\$!; $testCmd; echo \"\$(hostname) killing \$pgpid \$pfpid\"; sudo pkill --signal INT -P \$pgpid; sudo pkill --signal INT -P \$pfpid'"

ssh josiah@192.168.1.101 "$remotecmd" & testPID[0]=$!
sleep 1;
ssh josiah@192.168.1.102 "$remotecmd" & testPID[1]=$!
sleep 1;
ssh josiah@192.168.1.103 "$remotecmd" & testPID[2]=$!
sleep 1;
ssh josiah@192.168.1.104 "$remotecmd" & testPID[3]=$!

echo "Test PIDs are ${testPID[*]}. Waiting until they finish." >&2
wait ${testPID[0]}; echo "Test 1 finished."
wait ${testPID[1]}; echo "Test 2 finished."
wait ${testPID[2]}; echo "Test 3 finished."
wait ${testPID[3]}; echo "Test 4 finished."

echo "Getting perf files..." >&2
remotecmd="/bin/bash -c ''"
mkdir $expdir/server1 $expdir/server2 $expdir/server3 $expdir/server4
scp josiah@192.168.1.101:/tmp/$date.* $expdir/server1/
scp josiah@192.168.1.102:/tmp/$date.* $expdir/server2/
scp josiah@192.168.1.103:/tmp/$date.* $expdir/server3/
scp josiah@192.168.1.104:/tmp/$date.* $expdir/server4/

echo "Cleaning up power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$group&port=$port" --quiet

echo ""
echo "Closing power logger (PID=$loggerPID)..." >&2
/bin/kill -f -s INT $loggerPID

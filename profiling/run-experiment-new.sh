#!/bin/bash

dir=$( realpath $( dirname $0 ) )

ip="192.168.1.200"

exp=${1-none}
expArg=${2-none}

if [[ "x$exp" == "xnone" ]]; then
	ls $dir/experiments
	echo -n "Choose an experiment: " >&2
	read exp
fi
turboStatPath="/home/josiah/serverpower/utilities/turbostat/turbostat"

testPath="/home/josiah/serverpower/power_modulating_interfaces/$exp/record_$exp.sh"
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
mkdir -p $expdir
logfile=$(cygpath -w "$expdir/powerlog.log")

echo "Cleaning up old power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$logfile" --quiet

echo "Starting power stream..." >&2
wget -O - "http://$ip:8282/stream?command=start&length=10000&type=csvScaled&fields=01234567&address=$logfile" --quiet

#perfCmd="date +%s.%N > /tmp/${date}.perflog && echo \" \" >> /tmp/${date}.perflog && sudo perf stat -e cpu-cycles -e instructions -a -I 100 -x \\  -o /tmp/${date}.perflog --append"
turbostatCmd="date +%s.%N > /tmp/${date}.pgadglog && echo \" \" >> /tmp/${date}.pgadglog && sudo $turboStatPath -i 0.5 >> /tmp/${date}.pgadglog"
testCmd="date +%s.%N > /tmp/${date}.testlog && echo \" \" >> /tmp/${date}.testlog && $testPath >> /tmp/${date}.testlog"

remotecmd="/bin/bash -c '$turbostatCmd & pgpid=\$!; $testCmd; echo \"\$(hostname) killing \$pgpid\"; sudo pkill --signal INT -P \$pgpid'"

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

echo "Getting files..." >&2
mkdir $expdir/server1 $expdir/server2 $expdir/server3 $expdir/server4
scp josiah@192.168.1.101:/tmp/$date.* $expdir/server1/
scp josiah@192.168.1.102:/tmp/$date.* $expdir/server2/
scp josiah@192.168.1.103:/tmp/$date.* $expdir/server3/
scp josiah@192.168.1.104:/tmp/$date.* $expdir/server4/

echo "Cleaning up power streams..." >&2
wget -O - "http://$ip:8282/stream?command=stop&address=$logfile&type=csvScaled" --quiet

echo ""

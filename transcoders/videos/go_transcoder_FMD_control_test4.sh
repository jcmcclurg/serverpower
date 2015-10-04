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

mkfifo toCalcSetpoint1 toCalcSetpoint2 toIController1 toIController2

echo "begin test"
sudo $getFreq | tee > toCalcSetpoint1 toCalcSetpoint2 &
(avconv -i $vIn -r 30 -y $vOut; echo "q" ) > toCalcSetpoint1 & echo $! > avconv1.pid
avconv -i $vIn -r 30 -y $vOut > toCalcSetpoint2 & echo $! > avconv2.pid
#cat < toCalcSetpoint1 | $calcSetpoint -d 1 -M 34 -m 10 -B 10000 > calc1 & 
#cat < toCalcSetpoint2 | $calcSetpoint -d 0 -M 34 -m 10 > calc2 & 
cat < toCalcSetpoint1 | $calcSetpoint -d 1 -M 34 -m 18 -B 3000 -o $logPath/calcSet1Data.csv | tee > toIController1 calc1 & 
cat < toCalcSetpoint2 | $calcSetpoint -d 1 -M 34 -m 18 -B 3000 -o $logPath/calcSet2Data.csv | tee > toIController2 calc2 & 
sudo $power_gadget -e 150 | tee > toIController1 toIController2 &
cat < toIController1 | $integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $(cat avconv1.pid) &
cat < toIController2 | $integralController -s 28 -n 0 -x 1 -t 0.1 -k 0 -d 0 -u 10 | $insertDelays -U -d 0.5 -p $(cat avconv2.pid) &

echo "started test programs"

# multiple input to pipe:
# 	Wrong way: (cat < freqOut1 & cat < avOut1)
# 	Correct way: 
#		mkfifo ~/my_fifo
#		command1 > ~/my_fifo &
#		command2 > ~/my_fifo &
#		command3 < ~/my_fifo
 
#working named pipes:
#sudo ./power_gadget -e 150 | tee > pipe1 pipe2
#cat < pipe1
#cat < pipe2

# Make three named pipes for input/output around power_gadget:
# 	pipe0 can send "q" to power_gadget to quit properly. 
#		Use: echo "q" > pipe0
# 	pipe1 & pipe2 recieve power_gadget output to send to two integralControllers 
#		Use: cat < pipe(n)
# 	pg.pid: file with the PID of the power_gadget process
#		Use: kill $(cat pg.pig) 
#
# mkfifo pipe0 pipe1 pipe2
# cat pipe0 | sudo ../../rapl/power_gadget -e 50 | tee > pipe1 pipe2 & echo $! > pg.pid
#
# this does the same, but will echo "q" to the pipe1 & pipe2 after power_gadget exits
# cat pipe | (sudo ../../rapl/power_gadget -e 50; echo "q") | tee > pipe1 pipe2 & echo $! > pg.pid



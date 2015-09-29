#!/bin/bash
# single transcoder workload with throughput constraints
# same as 3 but with named pipes

:<<'COMMENT'
this gets ignored
COMMENT

dir="/home/powerserver/joe/serverpower"
vIn="$dir/transcoders/videos/cut.mp4"
vOut="$dir/transcoders/videos/output10.avi"
power_gadget="$dir/rapl/power_gadget"
calcSetpoint="$dir/xbeecom/RS-232-Lib/calcSetpoint"
logTranscoder="$dir/xbeecom/RS-232-Lib/test.csv"
integralController="$dir/integralController/integralController"
insertDelays="$dir/signal_insert_delays/insertDelays"
getFreq="$dir/xbeecom/RS-232-Lib/getFreq"

mkfifo toQuitGetFreq toCalcSetpoint1 toCalcSetpoint2 

echo "begin test"
cat < toQuitGetFreq | sudo $getFreq | tee > toCalcSetpoint1 toCalcSetpoint2 &
avconv -i $vIn -r 30 -y $vOut > toCalcSetpoint1 & echo $! > avconv1.pid
avconv -i $vIn -r 30 -y $vOut > toCalcSetpoint2 & echo $! > avconv2.pid
cat < toCalcSetpoint1 | $calcSetpoint > calc1 & 
cat < toCalcSetpoint2 | $calcSetpoint > calc2 & 

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



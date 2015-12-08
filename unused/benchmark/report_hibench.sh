#!/bin/bash


d=`date +%s.%N`

echo $d.out
echo "Starting power gadget at "`date +%s.%N` > $d.out
(sudo ../signal_insert_delays/power_gadget_whileloop/power_gadget -e 1000 -c tp > $d.power) & 
pgpid=$!

trap 'echo "Got interrupt"; echo "Stopping power gadget at "`date +%s.%N` >> '$d'.out; sudo kill -s SIGINT '$pgid SIGINT SIGTERM

./go_hibench.sh | tee -a $d.out
cp ../../HiBench/report/hibench.report $d.report

echo "Stopping power gadget at "`date +%s.%N` >> $d.out
sudo kill -s SIGINT $pgpid

echo "Done."
echo $d.out

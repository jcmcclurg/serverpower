#!/bin/bash

echo "Starting power gadget at "`date +%s.%N` >&2
sudo ../signal_insert_delays/power_gadget_whileloop/power_gadget -e 150 -c tp & 
pgpid=$!

sleep 5
echo "Starting stress at "`date +%s.%N` >&2
stress -c 4 &
spid=$!
sleep 5

trap "echo 'Stopping stress' >&2; pkill -P $spid; echo 'Stopping power gadget' >&2; sudo kill -s SIGINT $pgpid; exit;" SIGINT SIGTERM

./step_profile.sh | ./cpufreq.sh
sleep 5
echo "Stopping stress at "`date +%s.%N` >&2
pkill -P $spid

sleep 5
echo "Stopping power gadget at "`date +%s.%N` >&2
sudo kill -s SIGINT $pgpid

echo "Done." >&2

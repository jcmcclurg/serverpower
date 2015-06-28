#!/bin/bash

echo "Starting power gadget at "`date +%s.%N` >&2
sudo ./power_gadget_whileloop/power_gadget -e 150 -c tp & 
pgpid=$!

sleep 5
echo "Starting stress at "`date +%s.%N` >&2
stress -c 4 &
spid=$!
sleep 5

./step_profile.sh | sudo ./powerclamp.sh
sleep 5
echo "Stopping stress at "`date +%s.%N` >&2
pkill -P $spid

sleep 5
echo "Stopping power gadget at "`date +%s.%N` >&2
sudo kill -s SIGINT $pgpid

echo "Done." >&2

#!/bin/bash

echo "Starting power gadget at " `date +%s.%N` >&2
sudo ./power_gadget_whileloop/power_gadget -e 100 -c tp > ./power_output.txt & 
pgpid=$!

sleep 5
echo "Starting stress at " `date +%s.%N` >&2
stress -c 4 &
spid=$!
./step_profile.sh | ./insertDelays $spid
pkill -P $spid
sudo kill $pgpid

echo "Done." >&2

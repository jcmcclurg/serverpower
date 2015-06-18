#!/bin/bash

echo "Starting power gadget at " `date +%s.%N` >&2
sudo ./power_gadget_whileloop/power_gadget -e 51 -c tp & 
pgpid=$!

sleep 5
echo "Starting stress at " `date +%s.%N` >&2
stress -c 4 &
spid=$!
sleep 5
./step_profile.sh | ./insertDelays $spid -e $$ $PPID $BASHPID $pgid $@
sleep 5

pkill -P $spid
sudo kill $pgpid

echo "Done." >&2

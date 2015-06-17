#!/bin/bash

epids=${1:-excludedPIDs}

echo "Starting power gadget at "`date +%s.%N` >&2
sudo ./power_gadget_whileloop/power_gadget -e 150 -c tp & 
pgpid=$!

echo $$ >> $epids
echo $PPID >> $epids
echo $BASHPID >> $epids
echo $pgpid >> $epids

sleep 5
echo "Starting stress at "`date +%s.%N` >&2
stress -c 4 &
spid=$!
sleep 5

./step_profile.sh $epids | (sleep 1; ./insertDelays -e `cat $epids` $$ $PPID $BASHPID)
sleep 5
echo "Stopping stress at "`date +%s.%N` >&2
pkill -P $spid

sleep 5
echo "Stopping power gadget at "`date +%s.%N` >&2
sudo kill -s SIGINT $pgpid

echo "Done." >&2

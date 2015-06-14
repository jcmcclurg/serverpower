#!/bin/bash

epids=${1:-excludedPIDs}

echo "Starting power gadget at " `date +%s.%N` >&2
sudo ./power_gadget_whileloop/power_gadget -e 51 -c tp & 
pgpid=$!

echo $$ >> $epids
echo $PPID >> $epids
echo $BASHPID >> $epids
echo $pgpid >> $epids

sleep 5
./step_profile.sh $epids | (sleep 1; ./insertDelays -e `cat $epids` $$ $PPID $BASHPID)
sleep 5

sudo kill $pgpid

echo "Done." >&2

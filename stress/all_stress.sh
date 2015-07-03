#!/bin/bash

echo "Starting power gadget at "`date +%s.%N` >&2
sudo ../signal_insert_delays/power_gadget_whileloop/power_gadget -e 150 -c tp & 
pgpid=$!

sleep 5
echo "Starting stress at "`date +%s.%N` >&2

./step_profile.sh | (sleep 1; ./go_four_cstress.sh)

sleep 5
echo "Stopping power gadget at "`date +%s.%N` >&2
sudo kill -s SIGINT $pgpid

echo "Done." >&2

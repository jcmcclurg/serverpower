#!/bin/bash
dir=./research/serverpower/signal_insert_delays/power_gadget_whileloop
sudo $dir/power_gadget -e 1000 -p 0: | perl $dir/driveGnuPlots.pl 1 500 "Power"

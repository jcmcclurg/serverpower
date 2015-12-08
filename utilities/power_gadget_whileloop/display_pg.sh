#!/bin/bash

#(sudo ./power_gadget -e 100 & tee)
sudo ./power_gadget -e 100 -p 0: | perl driveGnuPlots.pl 1 500 "Power"

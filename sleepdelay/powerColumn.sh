#!/bin/bash

#./power_gadget -e 500 | cut -d ',' -f1
# for some reason, cut buffers the output, so you have to turn that off
stdbuf -i0 -o0 -e0 sudo ./power_gadget -e 40 | tee data/data.csv | stdbuf -i0 -o0 -e0 cut -d ',' -f3
#sudo ./power_gadget -e 1000 | stdbuf -i0 -o0 -e0 cut -d ',' -f5

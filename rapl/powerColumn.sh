#!/bin/bash

#./power_gadget -e 500 | cut -d ',' -f1
# for some reason, cut buffers the output, so you have to turn that off
stdbuf -i0 -o0 -e0 sudo ./power_gadget -e 500 | stdbuf -i0 -o0 -e0 cut -d ',' -f1,3-5,8,11,14 | tee data/data.csv | stdbuf -i0 -o0 -e0 cut -d ',' -f4,5
#sudo ./power_gadget -e 1000 | stdbuf -i0 -o0 -e0 cut -d ',' -f5

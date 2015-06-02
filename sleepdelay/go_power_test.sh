#!/bin/bash
# pwrtest gives setpoint | power_gadget pulls msr data and sets powerlimit | cut grabs PKG Power
#stdbuf -i0 -o0 -e0 ./pwrtest | sudo ./go_closed_loop.sh
(stdbuf -i0 -o0 -e0 ./pwrtest & tee) | stdbuf -i0 -o0 -e0 sudo ./power_gadget -e 100 | sudo python -u powerControlLoop.py | sudo python pythonStress.py
 

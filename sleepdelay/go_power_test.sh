#!/bin/bash
# pwrtest gives setpoint | power_gadget pulls msr data and sets powerlimit | cut grabs PKG Power
stdbuf -i0 -o0 -e0 ./pwrtest | sudo ./go_closed_loop.sh

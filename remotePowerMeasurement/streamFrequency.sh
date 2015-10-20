#!/bin/bash
dir="/home/powerserver/joe/serverpower/remotePowerMeasurement"
python -u $dir/measurementServer/multicast_listen.py -n -p 9998 -a 224.1.1.2

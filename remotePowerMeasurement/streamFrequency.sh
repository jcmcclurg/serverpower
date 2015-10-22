#!/bin/bash
#dir="/home/powerserver/joe/serverpower/remotePowerMeasurement"
dir=$( dirname $0 )
python -u $dir/measurementServer/multicast_listen.py -n -p 9501 -a 224.1.1.1

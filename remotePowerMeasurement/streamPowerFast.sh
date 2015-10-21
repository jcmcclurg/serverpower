#!/bin/bash

#dir=$( dirname $0 )
dir="/home/powerserver/joe/serverpower/remotePowerMeasurement"
python -u $dir/measurementServer/multicast_listen.py -n

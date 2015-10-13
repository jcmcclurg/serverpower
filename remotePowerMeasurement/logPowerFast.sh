#!/bin/bash

dir=$( dirname $0 )

$defLogfile="/tmp/powerLog.log"

logfile=$(1:-defLogfile)

echo "Logging power and timestamp to $logfile"
python -u $dir/measurementServer/multicast_listen.py -n > $defLogFile

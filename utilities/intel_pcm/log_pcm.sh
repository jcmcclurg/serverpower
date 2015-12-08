#!/bin/bash

date=$( date +%s.%N )
defLogfile="/tmp/pcm_${date}.log"
logfile=${1-$defLogfile}

echo "Logging to ${logfile}." >&2
sudo ./pcm.x 0.1 -r --nocores --nosockets -csv=$logfile

#!/bin/bash
( while true; do wget -O - -o /dev/null http://jjpowerserver2.ddns.net:18282/power?l=1666 | cut -d \  -f 2,3; sleep 0.150; done ) > powerLog.log & pid=$!;
tail --pid=$pid -s 0.1 -f powerLog.log | cut -d \  -f 2

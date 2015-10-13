#!/bin/bash
# 10000Sa/s * 60Hz = 166.66Sa/cycle  => 9cycles*166.66Sa/cycle = 1500 samples
( while true; do wget -O - -o /dev/null http://jjpowerserver2.ddns.net:18282/power?l=15000 | cut -d \  -f 2,4; sleep 0.150; done ) > powerLog.log & pid=$!;
tail --pid=$pid -s 0.150 -f powerLog.log | unbuffer -p awk -F [' '] '{printf("%.2f\n",$2)}'

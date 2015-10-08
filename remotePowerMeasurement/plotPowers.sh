#!/bin/bash

( while true; do wget -O - -o /dev/null http://jjpowerserver2.ddns.net:18282/power?l=10000 | cut -d \  -f 4,5,6,7,8; sleep 1; done ) | ./feedgnuplot/bin/feedgnuplot --line --legend 0 'jjpowerserver' --legend 1 'jjpowerserver1' --legend 2 'jjpowerserver2' --legend 3 'jjpowerserver3' --legend 4 'jjpowerserver4' --stream 0.1 --xlen 120

#!/bin/bash

# THIS FILE SHOULD NOT BE PUSHED! It's broken!

( while true; do wget -O - http://localhost:8282/power?l=1667 2> /dev/null | cut -d \  -f 4,5,6,7,8; sleep 0.167; done ) | ./feedgnuplot/bin/feedgnuplot --line --legend 0 'jjpowerserver' --legend 1 'jjpowerserver1' --legend 2 'jjpowerserver2' --legend 3 'jjpowerserver3' --legend 4 'jjpowerserver4' --stream 0.1 --xlen 120

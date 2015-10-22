#!/bin/bash
addr="192.168.1.200:8282"
port="9500"
port2="9501"
l=1000
# for type=power, fields are
# 0 - start time
# 1 - end time
# 2 - total power in watts
# 3 - server 0
# 4 - server 1
# 5 - server 2
# 6 - server 3
# 7 - server 4

# for type=freq, fields are
# 0 - start time
# 1 - end time
# 2 - frequency in hz


wget -O - "$addr/stream?command=start&length=$l&blockLength=$l&type=power&fields=01234567&address=224.1.1.1&port=$port" --quiet
wget -O - "$addr/stream?command=start&length=1&blockLength=1&type=freq&fields=02&address=224.1.1.1&port=$port2" --quiet

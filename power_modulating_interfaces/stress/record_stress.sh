#!/bin/bash

dir=$(dirname $0)
p=0.01
s=$dir/cstress

( ( $dir/../utilities/playback -f $dir/../utilities/ramp_0_1.csv; echo "q"; echo "q") | tee >($s -i 1: -p $p) >($s -i 2: -p $p) >($s -i 3: -p $p) >($s -i 4: -p $p) >($s -i 5: -p $p) >($s -i 6: -p $p) >($s -i 7: -p $p) >($s -i 8: -p $p) >($s -i 9: -p $p) >($s -i 10: -p $p) >($s -i 11: -p $p) >($s -i 12: -p $p) > /dev/null ) 2>&1
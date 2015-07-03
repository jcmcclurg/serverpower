#!/bin/bash

dir="./power_output"
d=`date +%N_%s`
echo $d

./all_stress.sh 2>"$dir/$d"".log" >"$dir/$d"".output"

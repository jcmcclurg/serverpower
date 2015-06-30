#!/bin/bash

dir="./power_output"
d=`date +%N_%s`
echo $d
#./stress_insertDelays.sh 2>"$dir/$d"".log" >"$dir/$d"".output"

./all_cpufreq.sh 2>"$dir/$d"".log" >"$dir/$d"".output"

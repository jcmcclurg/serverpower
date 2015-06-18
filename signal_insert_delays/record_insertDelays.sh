#!/bin/bash

dir="./power_output"
mkdir $dir
d=`date +%N_%s`
echo $d
#./stress_insertDelays.sh 2>"$dir/$d"".log" >"$dir/$d"".output"
excludedPIDs=${1:-excludedPIDs}

echo $$ > $excludedPIDs
echo $PPID >> $excludedPIDs
echo $BASHPID >> $excludedPIDs

./all_insertDelays.sh $excludedPIDs 2>"$dir/$d"".log" >"$dir/$d"".output"

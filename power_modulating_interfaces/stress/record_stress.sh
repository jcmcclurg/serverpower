#!/bin/bash

dir=$(dirname $0)
p=0.01
s=$dir/cstress

playbackCmd="$dir/../../utilities/playback/playback"

rangeMin=0
rangeMax=1
defRampFile="$dir/interleavedRamp_${rangeMin}_${rangeMax}.csv"
dummyRampFile="$dir/interleavedRamp_${rangeMin}_${rangeMax}_short.csv"

rampFile=${1-$defRampFile}

if [[ "x$rampFile" == "xnone" ]]; then
	rampFile=$defRampFile
elif [[ "x$rampFile" == "xdummy" ]]; then
	rampFile=$dummyRampFile
else
	rampFile="$dir/${rampFile}_${rangeMin}_${rangeMax}.csv"
fi

( ( $playbackCmd -v -f $rampFile; echo "q"; echo "q") | tee >($s -i 1: -p $p) >($s -i 2: -p $p) >($s -i 3: -p $p) >($s -i 4: -p $p) >($s -i 5: -p $p) >($s -i 6: -p $p) >($s -i 7: -p $p) >($s -i 8: -p $p) >($s -i 9: -p $p) >($s -i 10: -p $p) >($s -i 11: -p $p) >($s -i 12: -p $p) > /dev/null ) 2>&1

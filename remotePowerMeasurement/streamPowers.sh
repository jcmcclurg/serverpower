#!/bin/bash

addr="jjpowerserver2.ddns.net:18282"
#defDuration="0.150"
defDuration="0.500"
duration=${1:-$defDuration}
numCycles=$( printf "%.0f" $( bc <<< "scale=1; 60*$duration" ) )
numSamples=$( printf "%.0f" $( bc <<< "scale=1; $numCycles*10000/60" ) )

callsPerBlock=$( printf "%0.f" $( bc <<< "scale=1; 30/$duration" ) )

while true; do
	(
		for i in $( seq $callsPerBlock ); do
			echo "http://$addr/power?l=$numSamples"
		done
	) | wget -O - -i - --wait=$duration --quiet
	sleep $duration;
done

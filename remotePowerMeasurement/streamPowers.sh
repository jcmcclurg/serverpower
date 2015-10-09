#!/bin/bash

addr="jjpowerserver2.ddns.net:18282"
#defDuration="0.150"
defDuration="0.500"
duration=${1:-$defDuration}
echo $duration

numCycles=$( bc <<< "scale=1; 60*$duration" | tr -d '[[:space:]]' )
numCycles=$( printf "%.0f" $numCycles )

numSamples=$( bc <<< "scale=1; $numCycles*10000/60" | tr -d '[[:space:]]' )
numSamples=$( printf "%.0f" $numSamples )

callsPerBlock=$( bc <<< "scale=1; 30/$duration" | tr -d '[[:space:]]' )
callsPerBlock=$( printf "%.0f" $callsPerBlock )


while true; do
	(
		for i in $( seq $callsPerBlock ); do
			echo "http://$addr/power?l=$numSamples"
		done
	) | wget -O - -i - --wait=$duration --quiet
	sleep $duration;
done

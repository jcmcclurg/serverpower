#!/bin/bash

addr="jjpowerserver2.ddns.net:18282"
#defDuration="0.150"
defDuration="1"
defConnectionDuration="60"

duration=${1:-$defDuration}
connectionDuration=${2:-$defConnectionDuration}

numCycles=$( bc <<< "scale=1; 60*$duration" | tr -d '[[:space:]]' )
numCycles=$( printf "%.0f" $numCycles )

numSamples=$( bc <<< "scale=1; $numCycles*10000/60" | tr -d '[[:space:]]' )
numSamples=$( printf "%.0f" $numSamples )

callsPerConnection=$( bc <<< "scale=1; $connectionDuration/$duration" | tr -d '[[:space:]]' )
callsPerConnection=$( printf "%.0f" $callsPerConnection )

defBlockLen=$( bc <<< "scale=1; $numSamples/10" | tr -d '[[:space:]]' )
defBlockLen=$( printf "%.0f" $defBlockLen )
blockLen=${3:-$defBlockLen}

echo "Time between downloads: $duration, TCP connection duration: $connectionDuration, Download is broken up into $blockLen-sample blocks." >&2
echo "Number of samples per download: $numSamples = $numCycles cycles" >&2
echo "http://$addr/power?l=$numSamples&b=$blockLen" >&2
while true; do
	(
		for i in $( seq $callsPerConnection ); do
			echo "http://$addr/power?l=$numSamples&b=$blockLen"
		done
	) | wget -O - -i - --wait=$duration --quiet
	sleep $duration;
done

#!/bin/bash

dir=$(dirname $0)

remoteDir=$1
masterUser=$2

queryType=$3
algorithm=$4
dataset=$5

# Create the working directory
rm -r $remoteDir
mkdir -p $remoteDir

# Copy the appropriate template
cp "$dir/spark_$queryType-$algorithm-$dataset""_template.sh" $remoteDir/submit.sh

# Go to the working directory
cd $remoteDir

date +%s.%N > submit.time
/bin/bash submit.sh > submit.out 2> submit.err &

# Poll yarn once per second for up to 20 seconds to get the application ID
for x in $(seq 20); do
	echo "Waiting 1 second to get app ID..."
	sleep 1
	appId=$(yarn application -list | grep $masterUser | cut -f 1 2>/dev/null)
	if [ ! -z "$appId" ]; then
		break
	fi
done

# If you've obtained the application ID, write it to a file
if [ ! -z "$appId" ]; then
	echo "Started $appId."
	echo $appId > appId
else
	echo "Could not get application ID!"
fi

# Wait until the spark command completes.
wait

# If you know your application ID, you can get the yarn logs.
if [ -e appId ]; then
	# Poll yarn once per second for up to 20 seconds to get the application logs
	for x in $(seq 20); do
		echo "Waiting 1 second to get application logs..."
		sleep 1
		yarn logs -applicationId $appId > yarnLogs.log
		retVal=$?
		if [ $retVal == 0 ]; then
			break
		fi
	done

	# If you were able to get the yarn logs, you can get the spark logs.
	if [ $retVal == 0 ]; then
		wget --quiet --header="accept-encoding: gzip" -O sparkEvents.json.gz http://jjpowerserver0.jjcluster.net:8188/ws/v1/timeline/spark_event_v01/$appId
	fi
fi

#!/bin/bash

mydir=$(dirname "$BASH_SOURCE")

source "$mydir/constants.sh"
source "$mydir/functions_remoteControl.sh"

# Create a local directory for the power monitoring
source "$mydir/functions_powerMonitor.sh"
setupPowerMonitor

# Upload the files needed for running the driver.
echo "Uploading script files..." >&2
if [ "$1" == "test" ]; then
	echo "Deleting /tmp/uploads/test/results (leaving the other files alone)..." >&2
	runRemote "rm -r '/tmp/uploads/test/results'" "$driverLogin"
	remoteDir=$( upload "$mydir/*.sh" "$driverLogin" "/tmp/uploads/test" )
else
	if [ -f "$mydir/remoteDir" ]; then
		rdir=$(cat "$mydir/remoteDir")
		#echo "Deleting all the previous files in $rdir..." >&2
		#runRemote "bash rm -r '$rdir'" "$driverLogin"
		remoteDir=$( upload "$mydir/*.sh" "$driverLogin" "$rdir" )
	else
		remoteDir=$( upload "$mydir/*.sh" "$driverLogin" )
	fi

fi

echo "Uploading jar files..." >&2
upload "$mydir/*.jar" "$driverLogin" "$remoteDir"

echo "Uploading configuration files..." >&2
upload "$mydir/sparkConf/*" "$driverLogin" "$remoteDir/sparkConf"
if [ "$1" == "test" ]; then
	# Set up the spark configuration
	runRemote "bash '$remoteDir/sparkConf/setupSparkConf.sh' test" "$driverLogin"
else
	# Set up the spark configuration
	runRemote "bash '$remoteDir/sparkConf/setupSparkConf.sh'" "$driverLogin"
fi


# Write the remote dir, so the stage executor can know where to find things.
echo "$remoteDir" > "$mydir/remoteDir"


# Upload the files needed for starting and stopping the power caps.
uploadMultiple "$mydir/functions_powerCap.sh $mydir/go_powerCap.sh $mydir/setupPowerCap.sh" "${executorLoginList[*]}" "$remoteDir"

# Set up the power capping.
runRemoteMultiple "bash '$remoteDir/setupPowerCap.sh'" "${executorLoginList[*]}"

# Make sure there are no existing applications running
runRemote "bash '$remoteDir/go_cleanup.sh'" "$driverLogin"

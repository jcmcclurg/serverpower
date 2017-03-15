#!/bin/bash

mydir=$(dirname "$BASH_SOURCE")

source "$mydir/constants.sh"
source "$mydir/functions_remoteControl.sh"

# Create a local directory for the power monitoring
source "$mydir/functions_powerMonitor.sh"
setupPowerMonitor

# Upload the files needed for running the driver.
#remoteDir=$( upload "$mydir/*" "$driverLogin" "/tmp/uploads/1489436986.893493500" )
remoteDir=$( upload "$mydir/*" "$driverLogin" )

# Write the remote dir, so the stage executor can know where to find things.
echo "$remoteDir" > "$mydir/remoteDir"

# Set up the spark configuration
runRemote "bash '$remoteDir/sparkConf/setupSparkConf.sh'" "$driverLogin"

# Upload the files needed for starting and stopping the power caps.
uploadMultiple "$mydir/functions_powerCap.sh $mydir/go_powerCap.sh $mydir/setupPowerCap.sh" "${executorLoginList[*]}" "$remoteDir"

# Set up the power capping.
runRemoteMultiple "bash '$remoteDir/setupPowerCap.sh'" "${executorLoginList[*]}"

# Make sure there are no existing applications running
runRemote "bash '$remoteDir/go_cleanup.sh'" "$driverLogin"

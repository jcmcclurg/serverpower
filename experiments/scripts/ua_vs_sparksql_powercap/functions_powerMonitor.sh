#!/bin/bash

if [ -z "$FUNCTIONS_POWERMONITOR" ]; then
	FUNCTIONS_POWERMONITOR=1

	mydir=$( dirname "$BASH_SOURCE" )
	source "$mydir/constants.sh"
	source "$mydir/functions_misc.sh"

	#powerLogTmpDir="/cygdrive/f/josiah/tmp"
	logfile="$tmpDir/logfile.gz"
	windowsLogfile=$( windowsPath "$logfile" )

	powerAverager="$mydir/printAvgPower.py"
	powerLogDir="$mydir/powerData"
	powerSummary="$mydir/powerSummary.txt"
	powerMonitorIPAndPort="localhost:8282"

	# Usage: returnData contains useful information from the last executed function.
	returnData=""

	setupPowerMonitor() {
		echo "Creating directory $powerLogDir..." >&2
		mkdir -p "$powerLogDir"

		# TODO: Verify that the power monitor software is actually running...
	}

	# Usage: startPowerMonitor
	#
	startPowerMonitor() {
		echo "Cleaning up old power streams..." >&2
		wget -O - "http://$powerMonitorIPAndPort/stream?command=stop&address=$windowsLogfile" --quiet

		echo "Starting power stream..." >&2
		wget -O - "http://$powerMonitorIPAndPort/stream?command=start&length=10000&type=csvScaled&fields=01234567&address=$windowsLogfile" --quiet
	}

	# Usage: stopPowerMonitor stage
	#
	stopPowerMonitor() {
		local stage="$1"

		echo "Stopping power stream for stage $stage..." >&2
		wget -O - "http://$powerMonitorIPAndPort/stream?command=stop&address=$windowsLogfile&type=csvScaled" --quiet

		# Process the power log in the background
		local tmpFile="$tmpDir/stage$stage.gz"
		mv "$logfile" "$tmpFile"

		echo "Block averaging power log in background to save space..." >&2

		teeToLogs "$powerLogDir/avgPower$stage" python "$powerAverager" "$stage" "$tmpFile" "$powerLogDir/stage$stage.gz_1000.gz" 1000
	}

	shutdownPowerMonitor() {
		echo "Cleaning up temporary power log $logfile..." >&2
		rm "$logfile"
	}
fi

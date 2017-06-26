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
		wget -O - "http://$powerMonitorIPAndPort/stream?command=stopAll" --quiet

		echo "Starting power stream..." >&2
		wget -O - "http://$powerMonitorIPAndPort/stream?command=start&length=10000&type=csvScaled&fields=01234567&address=$windowsLogfile" --quiet
	}

	# Usage: stopPowerMonitor stage startTime endTime
	#
	stopPowerMonitor() {
		local stage="$1"
		local startTime="$2"
		local endTime="$3"

		echo "Stopping power stream for stage $stage ($startTime, $endTime)..." >&2
		wget -O - "http://$powerMonitorIPAndPort/stream?command=stop&address=$windowsLogfile&type=csvScaled" --quiet

		# Process the power log in the background
		local largeFile="$powerLogDir/stage$stage.gz"
		mv "$logfile" "$largeFile"

		echo "Block averaging power log in background to save space..." >&2

		teeToLogs "$powerLogDir/avgPower$stage" python "$powerAverager" "$stage" "$largeFile" "1000" "1.0" "$startTime" "$endTime"

		# Probably, we should just delete the large files after use, but I move them to the temporary directory (to be overwritten by the next experiment) just to be safe.
		mv "$largeFile" "$tmpDir/prevStage.gz"
	}

	shutdownPowerMonitor() {
		echo "Cleaning up temporary power log $logfile..." >&2
		rm "$logfile"
	}
fi

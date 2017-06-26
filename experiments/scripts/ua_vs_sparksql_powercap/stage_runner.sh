#!/bin/bash

mydir=$(dirname "$BASH_SOURCE")

if [ -z "$STAGE_RUNNER" ]; then
	remoteDir=$( cat "$mydir/remoteDir" )
	STAGE_RUNNER=1

	source "$mydir/functions_misc.sh"
	source "$mydir/functions_remoteControl.sh"
	source "$mydir/functions_powerMonitor.sh"


	stagecleanup() {
		if [ -z "$stagecleanedup" ]; then
			runRemote "bash '$remoteDir/go_cleanup.sh'" "$driverLogin"
			runRemoteMultiple "bash -- '$remoteDir/go_powerCap.sh' -1" "${executorLoginList[*]}"
			stopPowerMonitor "$stage"
			stagecleanedup=1
		fi
	}

	run() {
		local stage=$1
		local powerCap=$2
		stagecleanedup=


		local stageSuccess=2
		echo "Running stage $stage at power cap $powerCap remotely on $remoteDir" >&2


		# Start the power monitor.
		if startPowerMonitor; then

			# Start the power capping.
			if runRemoteMultiple "bash -- '$remoteDir/go_powerCap.sh' $powerCap" "${executorLoginList[*]}"; then

				trap stagecleanup INT

				# Run the stage remotely. Even if that doesn't work, you still need to shut things down.
				if runRemote "bash -- '$remoteDir/go_queries.sh' $*" "$driverLogin"; then
					stageSuccess=0
					download "$remoteDir/results/stageResult$stage.txt" "$driverLogin" "$mydir/results"
					startTime=$(grep "startTime" "$mydir/results/stageResult$stage.txt" | cut -d \  -f 2)
					endTime=$(grep "endTime" "$mydir/results/stageResult$stage.txt" | cut -d \  -f 2)
				else
					stageSuccess=$?
					stagecleanup
				fi
			else
				stageSuccess=$?
			fi

			# Stop the power capping
			runRemoteMultiple "bash -- '$remoteDir/go_powerCap.sh' -1" "${executorLoginList[*]}"
		else
			stageSuccess=$?
		fi

		# Stop the power monitor
		stopPowerMonitor "$stage" "$startTime" "$endTime"

		return $stageSuccess
	}
fi

run "$@"

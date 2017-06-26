#!/bin/bash

source "$(dirname "$BASH_SOURCE")/constants.sh"

if [ -z "$FUNCTIONS_MISC" ]; then
	FUNCTIONS_MISC=1

	# Use this command if you want to exit the script if the code does not succeed, in a manner similar to
	#  set -e
	#   or
	#  trap 'cmds' ERR
	#
	# Usage: doOrExit commands...
	doOrExit() {
		local cmd=$1
		shift
		if $cmd "$@"; then
			return 0
		else
			local ret=$?
			echo "EXITING! $* had error code $ret." >&2
			exit $ret
		fi
	}

	teeToLogs() {
		local logName=$1
		local cmd=$2
		shift 2

		$cmd "$@" > >(stdbuf -o L -e L tee "$logName-out.log") 2> >(stdbuf -o L -e L tee "$logName-err.log" >&2)
	}

	# Usage: windowsPath name
	#
	windowsPath() {
		local name=${1:-.}
		name=$(readlink -f "$name")
		cygpath -w "$name"
	}

	runInBackground() {
		local cmd="$1"
		shift
		local pid=
		local date=$( date +%s.%N )
		local tmpFile="$tmpDir/$date"

		(
			if $cmd "${@}"; then
				touch "$tmpFile"
			else
				echo "$cmd had an error ($?)" >&2
			fi
		) & pid=$!

		echo "$tmpFile" > "$tmpDir/$pid.pid"
	}

	waitOnPIDs() {
		echo "Waiting on PIDs $* to finish..." >&2
		wait "$@"

		echo "Getting status filenames from PIDs..." >&2
		local files=()
		local file=
		for pid in "$@"; do
			file=$( cat "$tmpDir/$pid.pid" )
			files+=("$file")
			rm "$tmpDir/$pid.pid"
		done

		echo "Checking success statuses..." >&2
		local ret=0
		for file in "${files[@]}"; do
			if [ ! -f "$file" ]; then
				echo "FAIL: $file not finished." >&2
				ret=2
			else
				rm "$file"
			fi
		done
		return $ret
	}
fi

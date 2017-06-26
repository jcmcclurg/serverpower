#!/bin/bash


if [ -z "$FUNCTIONS_REMOTECONTROL" ]; then
	FUNCTIONS_REMOTECONTROL=1
	source "$(dirname "$BASH_SOURCE")/functions_misc.sh"

	driverLogin="josiah@jjpowerserver0"
	executorLoginList=("josiah@jjpowerserver1" "josiah@jjpowerserver2" "josiah@jjpowerserver3" "josiah@jjpowerserver4")


	# Usage: upload localFile(s) user@remoteIP [remoteDir]
	# 	If you want to upload multiple files, just use something like
	#		/{dir1/f1.txt,dir2/f2.txt}
	#			or
	#		/dir1/*.txt
	#
	upload() {
		local localFiles="$1"
		local login="$2"

		local date=$( date +%s.%N )
		local defaultRemoteDir="$tmpDir/uploads/$date"
		local remoteDir=${3:-$defaultRemoteDir}
		remoteDir=$( readlink -fm "$remoteDir" )
		local ret=-1
		echo "Creating remote temporary directory $remoteDir on $login..." >&2
		if	ssh "$login" "mkdir -p '$remoteDir'" > /dev/null; then
			echo "Copying local files $localFiles to $login:$remoteDir..." >&2
			if scp -r $localFiles "$login:$remoteDir" > /dev/null; then
				echo "$remoteDir"
			else
				ret="$?"
				echo "ERROR! Could not copy local files $localFiles to $login:$remoteDir (process returned $ret)." >&2
				return "$ret"
			fi
		else
			ret="$?"
			echo "ERROR! Could not create remote directory $remoteDir (process returned $ret)." >&2
			return "$ret"
		fi
	}
	uploadMultiple() {
		local remoteFiles="$1"

		local logins=($2)
		local date=$( date +%s.%N )
		local defaultRemoteDir="$tmpDir/uploads/$date"
		local remoteDir=${3:-$defaultRemoteDir}
		remoteDir=$( readlink -fm "$remoteDir" )

		local pids=()
		local pid=
		for login in "${logins[@]}"; do
			runInBackground upload "$remoteFiles" "$login" "$remoteDir"
			pid="$!"
			pids+=("$pid")
		done

		waitOnPIDs "${pids[@]}"
	}

	# Usage: runRemote remoteCmd user@remoteIP
	#
	# Returns:
	# 	returnData=	""
	#
	runRemote() {
		local cmd="$1"
		local login="$2"
		local ret=-1

		echo "Running remote command $cmd on $login..." >&2
		if	ssh "$login" "$cmd"; then
			# do nothing
			:
		else
			ret="$?"
			echo "ERROR! Could not run $cmd on $login (process returned $ret)." >&2
			return "$ret"
		fi
	}
	runRemoteMultiple() {
		local cmd="$1"
		local logins=($2)

		local pids=()
		local pid=
		for login in "${logins[@]}"; do
			runInBackground runRemote "$cmd" "$login"
			pid=$!
			pids+=("$pid")
		done

		waitOnPIDs "${pids[@]}"
	}

	# Usage: download remoteFile(s) user@remoteIP localDir
	# 	If you want to download multiple files, just use something like
	#		/{dir1/f1.txt,dir2/f2.txt}
	#			or
	#		/dir1/*.txt
	#
	#
	download() {
		local remoteFiles="$1"
		local login="$2"
		local date=$( date +%s.%N )
		local defaultLocalDir="$tmpDir/downloads/$date"
		local localDir=${3:-$defaultLocalDir}
		localDir=$( readlink -fm "$localDir" )
		local ret=-1

		if mkdir -p "$localDir" > /dev/null; then
			echo "Downloading $remoteFiles from $login to $localDir..."
			if scp -r "$login:$remoteFiles" "$localDir" > /dev/null; then
				echo "$localDir"
			else
				ret="$?"
				echo "ERROR! Could not download $remoteFiles from $login (process returned $ret)." >&2
				return "$ret"
			fi
		else
			ret="$?"
			echo "ERROR! Could not create $localDir/$login on local machine (process returned $ret)." >&2
			return "$ret"
		fi
	}

	downloadMultiple() {
		local remoteFiles="$1"

		local logins=($2)
		local date=$( date +%s.%N )
		local defaultLocalDir="$tmpDir/downloads/$date"
		local localDir=${3:-$defaultLocalDir}
		localDir=$( readlink -fm "$localDir" )

		local pids=()
		local pid=
		for login in "${logins[@]}"; do
			runInBackground download "$remoteFiles" "$login" "$localDir/$login"
			pid="$!"
			pids+=("$pid")
		done

		waitOnPIDs "${pids[@]}"
	}
fi

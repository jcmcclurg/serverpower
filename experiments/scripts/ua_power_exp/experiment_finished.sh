#!/bin/bash

mydir=$(dirname "$BASH_SOURCE")

experimentSucceeded="$1"
stage="$2"
lastReturnVal="$3"


source "$mydir/functions_misc.sh"
source "$mydir/functions_remoteControl.sh"

remoteDir=$( cat "$mydir/remoteDir" )
logins="$driverLogin ${executorLoginList[*]}"

# Download the remote files.
download "$remoteDir/results/*" "$driverLogin" "$mydir/results"

# Create a summary of the stage parameters
bash "$mydir/stages.sh" header > "$mydir/stageInputs.txt"
bash "$mydir/stages.sh" >> "$mydir/stageInputs.txt"

# Process the remote files
echo "Cleaning up the empty files in $mydir..." >&2
find "$mydir" -type f -empty -delete

echo "Merging the stage result summary files..." >&2
stageFiles=$(ls -1 "$mydir"/results/stageResult*.txt)
stageFiles=( $stageFiles )
stageResult="$mydir/stageResultSummary.txt"
cut -d \  -f 1 "${stageFiles[0]}" > "$stageResult"

for stageFile in "${stageFiles[@]}"; do
cut -d \  -f 2 "$stageFile" | paste "$stageResult" - > "$stageResult.tmp"
mv "$stageResult.tmp" "$stageResult"
done

echo "Merging the power summary files..." >&2
powerFiles=$(ls -1 "$mydir"/powerData/avgPower*-out.log)
powerFiles=( $powerFiles )
powerResult="$mydir/powerResultSummary.txt"
cut -d \  -f 1 "${powerFiles[0]}" > "$powerResult"

for powerFile in "${powerFiles[@]}"; do
cut -d \  -f 2 "$powerFile" | paste "$powerResult" - > "$powerResult.tmp"
mv "$powerResult.tmp" "$powerResult"
done

source "$mydir/functions_powerMonitor.sh"
shutdownPowerMonitor

echo "Collecting the detailed stage summaries..." >&2
stageFiles=$( ls -1 "$mydir"/results/stage*-err.log )
stageFiles=( $stageFiles )
stageResult="$mydir/results/stageSummary"

ids=()
updateIDs=()
queryIDs=()

arr1=()
arr2=()
arr3=()

for stageFile in "${stageFiles[@]}"; do
	s=$( echo "$stageFile" | sed 's/.*stage\([0-9]\+\).*/\1/g' )
	echo "   Processing stage $s ($stageFile)..." >&2
	outFile="$stageResult$s.txt"
	#echo -e "id localID isUpdate startTime submittedTime finishTime duration percentAffected version" > "$outFile"

	while read line; do
		l=($line)
		id=${l[2]}
		if [ "${l[0]}" == "u" ]; then
			updateIDs[${l[1]}]=$id
			arr1[$id]=1
		else
			queryIDs[${l[1]}]=$id
			arr1[$id]=0
		fi
		arr2[$id]=${l[1]}
		ids[$id]=$id
	done < <(grep 'Thread submitted [uq][0-9]\+ (#[0-9]\+' "$stageFile" | sed 's/.*Thread submitted \([uq]\)\([0-9]\+\) (#\([0-9]\+\).*/\1 \2 \3/g')
	echo "id ${ids[*]}" > "$outFile"
	echo "isUpdate ${arr1[*]}" >> "$outFile"
	echo "qid ${arr2[*]}" >> "$outFile"

	for i in "${ids[@]}"; do
		arr1[$i]=""
	done
	while read line; do
		l=($line)
		if [ "${l[0]}" == "u" ]; then
			id=${updateIDs[${l[1]}]}
		else
			id=${queryIDs[${l[1]}]}
		fi
		arr1[$id]=${l[2]}
	done < <(grep '[uq][0-9]\+:.*started at [0-9]\+' "$stageFile" | sed -e 's/.*\([uq]\)\([0-9]\+\):.*started at \([0-9]\+\).*/\1 \2 \3/g')
	echo "startTime ${arr1[*]}" >> "$outFile"

	for i in "${ids[@]}"; do
		arr1[$i]=""
		arr2[$i]=""
	done
	while read line; do
		l=($line)
		if [ "${l[0]}" == "u" ]; then
			id=${updateIDs[${l[1]}]}
		else
			id=${queryIDs[${l[1]}]}
		fi
		arr1[$id]=${l[2]}
		arr2[$id]=${l[3]}
	done < <(grep '[uq][0-9]\+:.*will affect.* [0-9\.eE+\-]\+ %.*submitting at [0-9]' "$stageFile" | sed -e 's/.*\([uq]\)\([0-9]\+\):.*will affect.* \([0-9\.eE+\-]\+\) %.*submitting at \([0-9]\+\).*/\1 \2 \3 \4/g')
	echo "percentAffected ${arr1[*]}" >> "$outFile"
	echo "submittedTime ${arr2[*]}" >> "$outFile"

	for i in "${ids[@]}"; do
		arr1[$i]=""
		arr2[$i]=""
		arr3[$i]=""
	done
	while read line; do
		l=($line)
		if [ "${l[0]}" == "u" ]; then
			id=${updateIDs[${l[1]}]}
		else
			id=${queryIDs[${l[1]}]}
		fi
		arr1[$id]=${l[2]}
		arr2[$id]=${l[3]}
		arr3[$id]=${l[4]}
	done < <(grep '[uq][0-9]\+:.*[fF]inished in [0-9]\+ ms at [0-9]\+.*version is [0-9]\+' "$stageFile" | sed -e 's/.*\([uq]\)\([0-9]\+\):.*[fF]inished in \([0-9]\+\) ms at \([0-9]\+\).*version is \([0-9]\+\).*/\1 \2 \3 \4 \5/g')
	echo "duration ${arr1[*]}" >> "$outFile"
	echo "finishTime ${arr2[*]}" >> "$outFile"
	echo "version ${arr3[*]}" >> "$outFile"
	
done



#lastStage=$(bash "$mydir/stages.sh" | wc -l - | cut -d \  -f 1)
#lastStage=$[ $lastStage - 1 ]
#oIFS="$IFS"
#IFS=$'\n'
#read -d '' -r -a stages < <(bash "$resultDir/$localStageGenerator")
#IFS="$oIFS"

#downloadMultiple "$remoteDir/powerCapOutput/*" "${executorLoginList[*]}" "$mydir/powerCapOutput"

if [ "$4" == "test" ]; then
	echo "This was a test, so we are leaving $remoteDir on $driverLogin ${executorLoginList[*]}"
else
	# Delete temporary remote files.
	if [ "$experimentSucceeded" == 1 ]; then
		echo "Temporary files deleted from $remoteDir on $driverLogin ${executorLoginList[*]}." >&2
		runRemoteMultiple "rm -r '$remoteDir'" "$driverLogin ${executorLoginList[*]}"
	else
		echo "The experiment did not succeed. Temporary files were left in $remoteDir on $driverLogin ${executorLoginList[*]}." >&2

		runRemote "bash '$remoteDir/go_cleanup.sh'" "$driverLogin"
	fi

	baseDir=$(readlink -f "$mydir")
	baseDir=$(basename "$baseDir")
	dropboxDir="/cygdrive/c/Users/Josiah/Dropbox/Research/2017/DynamicBigData/paper/$baseDir"
	mkdir -p "$dropboxDir"
	cp $mydir/stageInputs.txt $mydir/stageResultSummary.txt $mydir/powerResultSummary.txt "$dropboxDir"
fi



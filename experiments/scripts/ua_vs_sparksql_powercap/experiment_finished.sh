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

# Process the remote files
echo "Cleaning up the empty files in $mydir..." >&2
find "$mydir" -type f -empty -delete

echo "Merging the stage result summary files..." >&2
stageFiles=$(ls -1 "$mydir"/results/stage*.txt)
stageFiles=( $stageFiles )
stageResult="$mydir/stageResultSummary.txt"
cut -d \  -f 1 "${stageFiles[0]}" > "$stageResult"

for stageFile in "${stageFiles[@]}"; do
	cut -d \  -f 2 "$stageFile" | paste "$stageResult" - > "$stageResult.tmp"
	mv "$stageResult.tmp" "$stageResult"
done

echo "Collecting the detailed stage summaries..." >&2
stageFiles=$( ls -1 "$mydir"/results/stage*-err.log )
stageFiles=( $stageFiles )
stageResult="$mydir/results/stageSummary"

ids=()
isUpdate=()
startTime=()
finishTime=()

for stageFile in "${stageFiles[@]}"; do
	s=$( echo "$stageFile" | sed 's/.*stage\([0-9]\+\).*/\1/g' )
	echo "   Processing stage $s ($stageFile)..." >&2
	outFile="$stageResult$s.txt"
	echo -e "id\tisUpdate\tstartTime\tfinishTime" > "$outFile"

	while read line; do
		id=$( echo "$line" | sed -e 's/.*[uUqQ]\([0-9]\+\):.*/\1/g' )
		if [[ "$line" =~ ^.*Update\ [0-9]+\ started\ .*at\ .*$ ]]; then
			#echo "found update start $id"
			ids[$id]="$id"
			isUpdate[$id]=1
			startTime[$id]=$(echo "$line" | sed -e 's/.* started .*at \([0-9]\+\).*/\1/g')
		elif [[ "$line" =~ ^.*Update\ [0-9]+\ finished\ .*\ at\ .*$ ]]; then
			#echo "found update finish $id"
			ids[$id]="$id"
			isUpdate[$id]=1
			finishTime[$id]=$(echo "$line" | sed -e 's/.* finished .* at \([0-9]\+\).*/\1/g')
		elif [[ "$line" =~ ^.*Running\ query\ .*\ at\ .*$ ]]; then
			#echo "found query start $id"
			ids[$id]="$id"
			isUpdate[$id]=0
			startTime[$id]=$(echo "$line" | sed -e 's/.*Running query .* at \([0-9]\+\).*/\1/g')
		elif [[ "$line" =~ ^.*Query\ returned\ .*\ from\ version\ .*\ at\ .*$ ]]; then
			#echo "found query finish $id"
			ids[$id]="$id"
			isUpdate[$id]=0
			finishTime[$id]=$(echo "$line" | sed -e 's/.*Query returned .* at \([0-9]\+\).*/\1/g')
		fi
	done < <(grep '[uUqQ][0-9]\+:' "$stageFile")
	
	for id in "${ids[@]}"; do
		if [ ! -z "$id" ]; then
			echo -e "$id\t${isUpdate[$id]}\t${startTime[$id]}\t${finishTime[$id]}" >> "$outFile"
		fi
	done

done



#lastStage=$(bash "$mydir/stages.sh" | wc -l - | cut -d \  -f 1)
#lastStage=$[ $lastStage - 1 ]
#oIFS="$IFS"
#IFS=$'\n'
#read -d '' -r -a stages < <(bash "$resultDir/$localStageGenerator")
#IFS="$oIFS"

#downloadMultiple "$remoteDir/powerCapOutput/*" "${executorLoginList[*]}" "$mydir/powerCapOutput"

# Delete temporary remote files.
if [ "$experimentSucceeded" == 1 ]; then
	echo "Temporary files deleted from $remoteDir on $driverLogin ${executorLoginList[*]}." >&2
	runRemoteMultiple "rm -r '$remoteDir'" "$driverLogin ${executorLoginList[*]}"
else
	echo "The experiment did not succeed. Temporary files were left in $remoteDir on $driverLogin ${executorLoginList[*]}." >&2

	runRemote "bash '$remoteDir/go_cleanup.sh'" "$driverLogin"
fi

source "$mydir/functions_powerMonitor.sh"
shutdownPowerMonitor

echo "Merging the power summary files..." >&2
powerFiles=$(ls -1 "$mydir"/powerData/avgPower*-out.log)
powerFiles=( $stageFiles )
powerResult="$mydir/powerResultSummary.txt"
cut -d \  -f 1 "${powerFiles[0]}" > "$powerResult"

for powerFile in "${powerFiles[@]}"; do
	cut -d \  -f 2 "$powerFile" | paste "$powerResult" - > "$powerResult.tmp"
	mv "$powerResult.tmp" "$powerResult"
done

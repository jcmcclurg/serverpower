#!/bin/bash

mydir=$(dirname "$BASH_SOURCE")

# Create a summary of the stage parameters
bash "$mydir/stages.sh" header > "$mydir/stageInputs.txt"
bash "$mydir/stages.sh" >> "$mydir/stageInputs.txt"

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

baseDir=$(readlink -f "$mydir")
baseDir=$(basename "$baseDir")
dropboxDir="/cygdrive/c/Users/Josiah/Dropbox/Research/2017/DynamicBigData/paper/$baseDir"
mkdir -p "$dropboxDir"
cp stageInputs.txt stageResultSummary.txt powerResultSummary.txt "$dropboxDir"

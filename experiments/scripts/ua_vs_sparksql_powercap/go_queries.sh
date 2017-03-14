#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )

source "$dir/functions_misc.sh"
originalDir=$(pwd)
echo "Changed directory to $dir." >&2
export SPARK_CONF_DIR=$(readlink -fm "$dir/sparkConf")

cd "$dir"
choices_numExecutors=(48 24 16 12 8 4)
choices_executorCores=(1 2 3 4 6 12)
choices_executorMemory=(640 1920 3200 4406 6737 13731)
parallelismIndex=4


# Usage: runQuerySet mode numQueries queryRate updateAttrSelectivity updateRowSelectivity
# 
# Where parallelismIndex is a number from 0 to 5 indicating how small to make the executors.
# 0 gives a large number of small executors 5 gives a small number of large executors.
#
runQuerySet(){
	local mode=$1
	local numQueries=$2
	local queryRate=$3
	local updateAttrSelectivity=$4
	local updateRowSelectivity=$5

	if [ "$mode" == 1 ]; then
		local class="com.jcmcclurg.updateaware.sql.SparkSQLTest"
	elif [ "$mode" == 0 ]; then
		local class="com.jcmcclurg.updateaware.bitmapindex.BitmapIndexTest"
	else
		local class="$mode"
	fi

	stdbuf -o L -e L \
		spark-submit \
		--master yarn \
		--deploy-mode client \
		--num-executors ${choices_numExecutors[$parallelismIndex]} \
		--executor-cores ${choices_executorCores[$parallelismIndex]} \
		--executor-memory ${choices_executorMemory[$parallelismIndex]}m \
		--class $class \
		queries-assembly-1.0.jar \
		--attributes-per-partition 8 \
		--max-bits-per-value 8 \
		--density 0.1 \
		--num-partitions 32 \
		--num-rows 1000000 \
		--merge-size 25000 \
		--update-rate 0 \
		--query-rate "$queryRate" \
		--query-attr-selectivity 0.5 \
		--update-attr-selectivity "$updateAttrSelectivity" \
		--update-row-selectivity "$updateRowSelectivity" \
		--max-select-size 200 \
		--num-queries "$numQueries" \
		--checkpoint-every 1 \
		--num-fences 0 \
		--update-ordering-mode random
		#--random-seed 555554444333221
}




stageIndex=$1
resultsdir="./results"
echo "Stage: $stageIndex" >&2

mkdir -p "$resultsdir"

function cleanup() {
	ret=${1:-$?}
	cd "$originalDir"
	echo "Need to clean up..." >&2
	hdfs dfs -ls /user/josiah/sparksql
	hdfs dfs -rm -r -skipTrash /user/josiah/sparksql/*
	echo "ERROR (code $ret)!" >&2
	exit $ret
}
trap cleanup INT

startTime=$( date +%s.%N )
stageResultFile="$resultsdir/stage$stageIndex.txt"
stageLog="$resultsdir/stage$stageIndex"
# $1=stageNum, $2=powerCap. These are not needed by runQuerySet
shift 2
if teeToLogs "$stageLog" runQuerySet "$@"; then
	endTime=$( date +%s.%N )
	echo "stage $stageIndex" > "$stageResultFile"
	echo "startTime $startTime" >> "$stageResultFile"
	echo "endTime $startTime" >> "$stageResultFile"
	grep "Measured average" "$stageLog-err.log" | sed 's/.*Measured average \([a-zA-Z]\+\): \([0-9]\+\.\?[0-9]*\) .*/\1 \2/g' >> "$stageResultFile"
	cd "$originalDir"

	echo "Success!" >&2
else
	ret=$?
	endTime=$( date +%s.%N )
	cleanup $ret
fi

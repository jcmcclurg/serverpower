#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )

source "$dir/functions_misc.sh"
originalDir=$(pwd)
echo "Changed directory to $dir." >&2
export SPARK_CONF_DIR=$(readlink -fm "$dir/sparkConf")

# Only needed if you use a custom version of spark, with hadoop-provided
export SPARK_DIST_CLASSPATH=$(hadoop classpath)

cd "$dir"
choices_numExecutors=(48 24 16 12 8 4)
choices_executorCores=(1 2 3 4 6 12)
choices_executorMemory=(640 1920 3200 4406 6737 13731)
parallelismIndex=5


# Usage: runQuerySet isSparkSQL numQueries isQueryTimingImmediate queryTimingSeparation isUpdateAggregationImmediate updateAggregationSeparation updateRowRate mergeSize numFences
# 
# Where parallelismIndex is a number from 0 to 5 indicating how small to make the executors.
# 0 gives a large number of small executors 5 gives a small number of large executors.
#
runQuerySet(){
	local isSparkSQL=$1
	local numQueries=$2
	local isQueryTimingImmediate=$3
	local queryTimingSeparation=$4

	local isUpdateAggregationImmediate=$5
	local updateAggregationSeparation=$6
	local useUpdateAdmissionControl=$7
	local updateRowRate=$8
	shift 8

	local mergeSize=$1
	local numFences=$2
	local density=$3
	local checkpointEvery=$4
	local numValues=$5
	local numAttributes=$6
	local numRows=$7
	local numPartitions=$8

	if [ "$isSparkSQL" == 1 ]; then
		local class="com.jcmcclurg.updateaware.sql.SparkSQLTest"
	elif [ "$isSparkSQL" == 0 ]; then
		local class="com.jcmcclurg.updateaware.bitmapindex.BitmapIndexTest"
	else
		local class="$isSparkSQL"
	fi

	if [ "$isQueryTimingImmediate" == 1 ]; then
		local queryTimingType="immediate"
	elif [ "$isQueryTimingImmediate" == 0 ]; then
		local queryTimingType="exponential"
	else
		local queryTimingType="constant"
	fi

	if [ "$isUpdateAggregationImmediate" == 1 ]; then
		local updateAggregationTimingType="immediate"
	elif [ "$isUpdateAggregationImmediate" == 0 ]; then
		local updateAggregationTimingType="exponential"
	else
		local updateAggregationTimingType="constant"
	fi

	if [ "$useUpdateAdmissionControl" == 1 ]; then
		local admissionControlType="probability"
	else
		local admissionControlType="rate"
	fi

	stdbuf -o L -e L \
		/home/josiah/spark-2.1.1-bin-without-hadoop/bin/spark-submit \
		--master yarn \
		--deploy-mode client \
		--num-executors "${choices_numExecutors[$parallelismIndex]}" \
		--executor-cores "${choices_executorCores[$parallelismIndex]}" \
		--executor-memory "${choices_executorMemory[$parallelismIndex]}m" \
		--conf "spark.sql.shuffle.partitions=$numPartitions" \
		--class "$class" \
		queries-assembly-2.11-1.1.jar \
		--num-rows "$numRows" \
		--checkpoint-every "$checkpointEvery" \
		--num-attributes "$numAttributes" \
		--num-values "$numValues" \
		--density "$density" \
		--num-partitions "$numPartitions" \
		--num-queries "$numQueries" \
		--max-select-size 1000 \
		--max-schema-size 300 \
		--query-timing-type "$queryTimingType" \
		--query-timing-param "$queryTimingSeparation" \
		--query-initial-attrs 1 \
		--query-other-attr-probability 0.5 \
		--update-aggregation-timing-type "$updateAggregationTimingType" \
		--update-aggregation-timing-param "$updateAggregationSeparation" \
		--update-row-$admissionControlType "$updateRowRate" \
		--merge-size "$mergeSize" \
		--num-fences "$numFences"

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
stageResultFile="$resultsdir/stageResult$stageIndex.txt"
stageLog="$resultsdir/stage$stageIndex"
# $1=stageNum, $2=powerCap. These are not needed by runQuerySet
shift 2
if teeToLogs "$stageLog" runQuerySet "$@"; then
	endTime=$( date +%s.%N )
	echo "stage $stageIndex" > "$stageResultFile"
	echo "submitTime $startTime" >> "$stageResultFile"
	echo "exitTime $endTime" >> "$stageResultFile"
	grep "QUERY_RESULT: " "$stageLog-err.log" | sed 's/.*QUERY_RESULT: \([a-zA-Z]\+\): \([0-9]\+\.\?[0-9eE\-]*\) .*/\1 \2/g' >> "$stageResultFile"
	cd "$originalDir"

	echo "Success!" >&2
else
	ret=$?
	endTime=$( date +%s.%N )
	cleanup $ret
fi

#!/bin/bash

if [ -z "$STAGES_FILE" ]; then
	STAGES_FILE=1

	stageParametersHeader() {
		echo "stage powerCap isSparkSQL numQueries isQueryTimingImmediate queryTimingSeparation isUpdateAggregationImmediate updateAggregationSeparation useUpdateAdmissionControl updateRowRateOrProbability mergeSize numFences valDensity checkpointEvery numValues numAttributes numRows numPartitions updatePieces queryOtherAttrProb"
	}

	stageParametersTest() {
		local stage=0
		local powerCap=
		local isSparkSQL=0

		local numQueries=50
		local isQueryTimingImmediate=0
		local queryTimingSeparation=10
		local isUpdateAggregationImmediate=1
		local updateAggregationSeparation=0
		local useUpdateAdmissionControl=1
		local updateRowRate=
		local mergeSize=
		local numFences=
		local density=0.99
		local checkpointEvery=1
		local numValues=100
		local numAttributes=100
		local numRows=1000000
		local numPartitions=44
		local updatePieces=1
		local queryOtherAttrProb=0.1


		for numFences in 0 16; do
			for mergeSize in 100 0; do
				for updateRowRate in 0.0001 0.001 0.01 0.02; do
					for powerCap in 65; do
						echo "$stage $powerCap $isSparkSQL $numQueries $isQueryTimingImmediate $queryTimingSeparation $isUpdateAggregationImmediate $updateAggregationSeparation $useUpdateAdmissionControl $updateRowRate $mergeSize $numFences $density $checkpointEvery $numValues $numAttributes $numRows $numPartitions $updatePieces $queryOtherAttrProb"
						stage=$[ $stage + 1 ]
					done
				done
			done
		done
	}

	expID="Jun17_sql_maxThroughput"

	stageParameters() {
		local stage=0
		local powerCap=
		local isSparkSQL=

		local numQueries=100
		local isQueryTimingImmediate=1
		local queryTimingSeparation=0
		local isUpdateAggregationImmediate=1
		local updateAggregationSeparation=0
		local useUpdateAdmissionControl=1
		local updateRowRate=
		local mergeSize=100
		local numFences=0
		local density=0.99
		local checkpointEvery=1
		local numValues=100
		local numAttributes=100
		local numRows=1000000
		local numPartitions=44
		local updatePieces=1
		local queryOtherAttrProb=0.1


		#for rep in 0 1; do
		#	for numFences in 0 16; do
		#		for mergeSize in 100 0; do
		#			for updateRowRate in 0.001; do
		#				for powerCap in -1 95 90 85 80 75 70 65; do
		#					echo "$stage $powerCap $isSparkSQL $numQueries $isQueryTimingImmediate $queryTimingSeparation $isUpdateAggregationImmediate $updateAggregationSeparation $useUpdateAdmissionControl $updateRowRate $mergeSize $numFences $density $checkpointEvery $numValues $numAttributes $numRows $numPartitions $updatePieces $queryOtherAttrProb"
		#					stage=$[ $stage + 1 ]
		#				done
		#			done
		#		done
		#	done
		#done

		for rep in 0 1 2 3; do
			for updateRowRate in 0.0001 0.001 0.01 0.02; do
				for isSparkSQL in 1; do
					for powerCap in -1 65; do
						echo "$stage $powerCap $isSparkSQL $numQueries $isQueryTimingImmediate $queryTimingSeparation $isUpdateAggregationImmediate $updateAggregationSeparation $useUpdateAdmissionControl $updateRowRate $mergeSize $numFences $density $checkpointEvery $numValues $numAttributes $numRows $numPartitions $updatePieces $queryOtherAttrProb"
						stage=$[ $stage + 1 ]
					done
				done
			done
		done
	}
fi

if [ "$1" == "header" ]; then
	stageParametersHeader
elif [ "$1" == "test" ]; then
	stageParametersTest
elif [ "$1" == "expID" ]; then
	echo "$expID"
elif [ "$1" == "" ] || [ "$1" == "stageParams" ]; then
	stageParameters
else
	echo "Invalid argument $1" >&2
fi

#!/bin/bash

if [ -z "$STAGES_FILE" ]; then
	STAGES_FILE=1
	maxParallelismIndex=5
	# There are 12 cores per server, and four servers
	choices_numExecutors=(48 24 16 12 8 4)
	choices_executorCores=(1 2 3 4 6 12)

	# Each server has 14.75G = 15104M, with max(384,0.1*amount requested) allocated for overhead.
	# See SparkExecutorCalculations.xlsx
	choices_executorMemory=(640 1920 3200 4406 6737 13731)

	maxStageIndex=$[ 2*2*2 - 1 ]

	stageParametersHeader() {
		echo -e "stage\tpowerCap\tisSparkSQL\tnumQueries\tqueryRate\tupdateAttrSelectivity\tupdateRowSelectivity"
	}

	stageParameters() {
		local stageCount=0
		local numRows=
		local selectivity=
		local updateAttrSelectivity=
		local updateRowSelectivity=
		local isSparkSQL=
		local numQueries=
		local queryRate=
		local powerCap=

		for selectivity in "0.2" "0.5" "1.0"; do
			updateAttrSelectivity=$selectivity
			updateRowSelectivity=$selectivity
			for isSparkSQL in 0 1; do
				if [ "$isSparkSQL" == "1" ]; then
					numQueries=50
					queryRate=0.01

				else
					numQueries=50
					queryRate=0.1
				fi

				for powerCap in 25 75 -1; do
					echo "$stageCount $powerCap $isSparkSQL $numQueries $queryRate $updateAttrSelectivity $updateRowSelectivity"
					stageCount=$[ $stageCount + 1 ]
				done
			done
		done
	}
fi

if [ "$1" == "header" ]; then
	stageParametersHeader
else
	stageParameters
fi

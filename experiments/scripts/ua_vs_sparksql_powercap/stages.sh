#!/bin/bash

if [ -z "$STAGES_FILE" ]; then
	STAGES_FILE=1

	stageParametersHeader() {
		echo "stage powerCap isSparkSQL numQueries isQueryTimingImmediate queryTimingSeparation isUpdateAggregationImmediate updateAggregationSeparation useUpdateAdmissionControl updateRowRateOrProbability mergeSize numFences valDensity checkpointEvery numValues numAttributes numRows numPartitions"
	}

	stageParametersTest() {
		#local numAttrs=256
		#local dbSize=$[ $numRows * $numAttrs ]

		local stage=0
		local powerCap=-1
		local isSparkSQL=
		local numQueries=1000
		local isQueryTimingImmediate=1
		local queryTimingSeparation=0
		local isUpdateAggregationImmediate=1
		local updateAggregationSeparation=0
		local useUpdateAdmissionControl=1

		local updateRowRate=0.0001

		local mergeSize=
		local numFences=
		local density=0.99
		local checkpointEvery=1
		local numValues=100
		local numAttributes=100
		local numRows=1000000
		local numPartitions=32

		for isSparkSQL in 0; do
			if [ "$isSparkSQL" == "1" ]; then
				mergeSize=0
				numFences=0

				echo "$stage $powerCap $isSparkSQL $numQueries $isQueryTimingImmediate $queryTimingSeparation $isUpdateAggregationImmediate $updateAggregationSeparation $useUpdateAdmissionControl $updateRowRate $mergeSize $numFences $density $checkpointEvery $numValues $numAttributes $numRows $numPartitions"
				stage=$[ $stage + 1 ]
			else
				mergeSize=500
				numFences=0

				echo "$stage $powerCap $isSparkSQL $numQueries $isQueryTimingImmediate $queryTimingSeparation $isUpdateAggregationImmediate $updateAggregationSeparation $useUpdateAdmissionControl $updateRowRate $mergeSize $numFences $density $checkpointEvery $numValues $numAttributes $numRows $numPartitions"
				stage=$[ $stage + 1 ]
			fi
		done
	}

	#expID="May28_maxThroughput_pcap_-1_85_65"
	expID="May30_powerTest2_pcap_-1_85_65"
	stageParameters() {
		local stage=0
		local powerCap=
		local isSparkSQL=
		local numQueries=200
		local isQueryTimingImmediate=
		local queryTimingSeparation=
		local isUpdateAggregationImmediate=
		local updateAggregationSeparation=
		local useUpdateAdmissionControl=
		local updateRowRate=
		local mergeSize=
		local numFences=
		local density=0.99
		local checkpointEvery=1
		local numValues=100
		local numAttributes=100
		#local density=0.1

		local numRows=1000000
		local numPartitions=32
		local updatesBetweenMerge=
		local us=
		local rep=

		local updateAggregationImmediates=
		local updateRowRates=
		local updateRowRates=
		local powerCaps=
		local mergeSizes=
		local numFences=
		local expType="2"
		if [ "$expType" == "1" ]; then
			useUpdateAdmissionControl=1
			isQueryTimingImmediate=1
			

			updateRowRates=(0.000001 0.000003 0.000006 0.00001 0.00003 0.00006 0.0001 0.0003 0.0006 0.001 0.003 0.006 0.01 0.03 0.06 0.1 0.3)
			fpUpdateRowRates=(0.000001 0.000003 0.000006 0.00001 0.00003 0.00006 0.0001 0.0003 0.0006 0.001 0.003 0.006 0.01)
			updateAggregationImmediates=(1)
			powerCaps=(-1 85 65)
			mergeSizes=(0 20)
			fpMergeSizes=(0 20)
			numFencesList=(0 16)
			isSparkSQLs=(0)

			queryTimingSeparations=(1.3)
			updateAggregationSeparations=(10.5)
			sparkSQLQueryTimingSeparations=(1.3)
			sparkSQLUpdateAggregationSparations=(10.5)

		elif [ "$expType" == "2" ]; then
			useUpdateAdmissionControl=0
			# exp=0, immediate=1, constant=2
			isQueryTimingImmediate=0

			# immediate=1, constant=2
			updateAggregationImmediates=(1 2)
			updateRowRates=("9.52381e-5")
			fpUpdateRowRates=("9.52381e-5")
			powerCaps=(-1 85 65)
			mergeSizes=(0 20)
			fpMergeSizes=(0 20)
			numFencesList=(0 16)
			isSparkSQLs=(0)
			queryTimingSeparations=(10.5)
			updateAggregationSeparations=(10.5)

			sparkSQLQueryTimingSeparations=(1.3)
			sparkSQLUpdateAggregationSparations=(10.5)
		fi

		for powerCap in "${powerCaps[@]}"; do
			for rep in 1 2; do
				for isUpdateAggregationImmediate in "${updateAggregationImmediates[@]}"; do
					for isSparkSQL in "${isSparkSQLs[@]}"; do
						if [ "$isSparkSQL" == "1" ]; then
							_numFencesList=(0)
							if [ "$isQueryTimingImmediate" == "1" ]; then
								_queryTimingSeparations=(0)
							else
								_queryTimingSeparations=("${sparkSQLQueryTimingSeparations[@]}")
							fi

							if [ "$isUpdateAggregationImmediate" == "1" ]; then
								_updateAggregationSeparations=(0)
							else
								_updateAggregationSeparations=("${sparkSQLUpdateAggregationSeparations[@]}")
							fi
						else
							_numFencesList=("${numFencesList[@]}")
							if [ "$isQueryTimingImmediate" == "1" ]; then
								_queryTimingSeparations=(0)
							else
								_queryTimingSeparations=("${queryTimingSeparations[@]}")
							fi

							if [ "$isUpdateAggregationImmediate" == "1" ]; then
								_updateAggregationSeparations=(0)
							else
								_updateAggregationSeparations=("${updateAggregationSeparations[@]}")
							fi
						fi

						for updateAggregationSeparation in "${_updateAggregationSeparations[@]}"; do
							for queryTimingSeparation in "${_queryTimingSeparations[@]}"; do
								for numFences in "${_numFencesList[@]}"; do
									if [ "$numFences" == "0" ]; then
										_updateRowRates=("${updateRowRates[@]}")
										if [ "$isSparkSQL" == "1" ]; then
											_mergeSizes=(0)
										else
											_mergeSizes=("${mergeSizes[@]}")
										fi
									else
										_updateRowRates=("${fpUpdateRowRates[@]}")
										_mergeSizes=("${fpMergeSizes[@]}")
									fi

									for updateRowRate in "${_updateRowRates[@]}"; do
										for mergeSize in "${_mergeSizes[@]}"; do
											echo "$stage $powerCap $isSparkSQL $numQueries $isQueryTimingImmediate $queryTimingSeparation $isUpdateAggregationImmediate $updateAggregationSeparation $useUpdateAdmissionControl $updateRowRate $mergeSize $numFences $density $checkpointEvery $numValues $numAttributes $numRows $numPartitions"
											stage=$[ $stage + 1 ]
										done
									done
								done
							done
						done
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

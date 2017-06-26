#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )

bash $dir/go_cleanup.sh
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

/home/josiah/spark-2.1.0-bin-without-hadoop/bin/spark-shell \
	--jars queries-assembly-2.11-1.1.jar \
	--master yarn \
	--deploy-mode client \
	--num-executors "${choices_numExecutors[$parallelismIndex]}" \
	--executor-cores "${choices_executorCores[$parallelismIndex]}" \
	--executor-memory "${choices_executorMemory[$parallelismIndex]}m" \

cd "$originalDir"

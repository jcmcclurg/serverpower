#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )

originalDir=$(pwd)
echo "Changed directory to $dir." >&2
export SPARK_CONF_DIR=$(readlink -fm "$dir/sparkConf")

# Only needed if you use a custom version of spark, with hadoop-provided
export SPARK_DIST_CLASSPATH=$(hadoop classpath)

cd "$dir"

/home/josiah/spark-2.1.0-bin-without-hadoop/sbin/start-history-server.sh

cd "$originalDir"

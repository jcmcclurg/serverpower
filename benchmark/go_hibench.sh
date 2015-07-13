#!/bin/bash

echo "Formatting namenode at "`date +%s.%N`
../../hadoop-2.7.0/bin/hdfs namenode -format 2>&1

echo "Starting DFS at "`date +%s.%N`
../../hadoop-2.7.0/sbin/start-dfs.sh 2>&1

echo "Starting YARN at "`date +%s.%N`
../../hadoop-2.7.0/sbin/start-yarn.sh 2>&1

echo "Starting HiBench at "`date +%s.%N`
../../HiBench/bin/run-all.sh 2>&1

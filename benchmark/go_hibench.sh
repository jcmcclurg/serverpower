#!/bin/bash

echo "Stopping YARN and DFS at "`date +%s.%N`
../../hadoop-2.7.1/sbin/stop-yarn.sh 2>&1
../../hadoop-2.7.1/sbin/stop-dfs.sh 2>&1

echo "Formatting namenode at "`date +%s.%N`
../../hadoop-2.7.1/bin/hdfs namenode -format 2>&1

echo "Starting DFS and YARN at "`date +%s.%N`
../../hadoop-2.7.1/sbin/start-dfs.sh 2>&1
../../hadoop-2.7.1/sbin/start-yarn.sh 2>&1

echo "Starting HiBench at "`date +%s.%N`
free -m
../../HiBench/bin/run-all.sh 2>&1
free -m

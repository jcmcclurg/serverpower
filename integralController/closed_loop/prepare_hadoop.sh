#!/bin/bash

dir=$(dirname $0)

echo "Formatting namenode at "`date +%s.%N`
$dir/../../../hadoop-2.7.0/bin/hdfs namenode -format

echo "Starting DFS at "`date +%s.%N`
$dir/../../../hadoop-2.7.0/sbin/start-dfs.sh

echo "Starting YARN at "`date +%s.%N`
$dir/../../../hadoop-2.7.0/sbin/start-yarn.sh 

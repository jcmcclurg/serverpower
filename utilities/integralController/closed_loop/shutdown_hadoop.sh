#!/bin/bash

dir=$(dirname $0)

echo "Stopping YARN at "`date +%s.%N`
$dir/../../../hadoop-2.7.0/sbin/stop-yarn.sh 

echo "Stopping DFS at "`date +%s.%N`
$dir/../../../hadoop-2.7.0/sbin/stop-dfs.sh

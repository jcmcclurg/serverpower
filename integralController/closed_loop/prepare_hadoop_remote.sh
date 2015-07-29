#!/bin/bash

addr="joe@"$(cat ~/ubud1_ip)
dir=/home/joe/research

echo "Formatting namenode at "`date +%s.%N`
ssh $addr "$dir/hadoop-2.7.0/bin/hdfs namenode -format"

echo "Starting DFS at "`date +%s.%N`
ssh $addr "$dir/hadoop-2.7.0/sbin/start-dfs.sh"

echo "Starting YARN at "`date +%s.%N`
ssh $addr "$dir/hadoop-2.7.0/sbin/start-yarn.sh"

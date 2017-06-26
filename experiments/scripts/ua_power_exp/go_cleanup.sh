#!/bin/bash

pids=()
appIDs=$(yarn application -list | cut -f 1 | tail -n +3)

echo "Killing apps $appIDs""..." >&2

for i in $appIDs; do
	yarn application -kill $i & pids+=($!)
done

for pid in "${pids[@]}"; do
	wait $pid
done

echo "Cleaning up /user/josiah/sparksql directory"
hdfs dfs -ls /user/josiah/sparksql
hdfs dfs -rm -r -skipTrash /user/josiah/sparksql/*

echo "Done!" >&2

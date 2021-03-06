#!/bin/bash

higgsNum=${1:-DEFAULT_HIGGS_NUM}

export HADOOP_OPTS=-Djava.library.path=/usr/hdp/current/hadoop-client/lib/native

/usr/hdp/2.3.4.0-3485/spark/bin/spark-submit \
--class knn.KnnArray \
--master yarn \
--deploy-mode client \
--num-executors 8 \
--executor-cores 6 \
--driver-memory 5g \
--executor-memory 5g \
--conf spark.scheduler.minRegisteredResourcesRatio=1.0 \
--conf spark.scheduler.maxRegisteredResourcesWaitingTime=5m \
/home/gguzun/spark/sparkHybrid.jar "gguzun/data/indexed/knn/array/higgs_""$higgsNum"".rdd" /home/gguzun/data/HIGGS_100.csv \
gguzun/data/indexed/knn/array/classDir/higgs.rdd 5 0 0 1 $higgsNum 20

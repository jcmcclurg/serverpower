#!/bin/bash

export HADOOP_OPTS=-Djava.library.path=/usr/hdp/current/hadoop-client/lib/native

/usr/hdp/2.3.4.0-3485/spark/bin/spark-submit \
--class powerPaper.KnnBsiImages \
--master yarn \
--deploy-mode client \
--num-executors 4 \
--executor-cores 12 \
--driver-memory 1g \
--executor-memory 10g \
--conf spark.scheduler.minRegisteredResourcesRatio=1.0 \
--conf spark.scheduler.maxRegisteredResourcesWaitingTime=5m \
/home/gguzun/spark/sparkHybrid.jar gguzun/data/indexed/knn/images100.rdd gguzun/data/queryKnn_1K.csv 5 243 0 15 0 0 /home/gguzun/data/classDir/images100.bsi 243 0 242 20

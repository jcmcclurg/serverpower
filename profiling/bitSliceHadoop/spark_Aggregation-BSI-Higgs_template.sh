#!/bin/bash

export HADOOP_OPTS=-Djava.library.path=/usr/hdp/current/hadoop-client/lib/native

/usr/hdp/2.3.4.0-3485/spark/bin/spark-submit \
--class powerPaper.SumAggregateBsi \
--master yarn \
--deploy-mode client \
--num-executors 4 \
--executor-cores 12 \
--driver-memory 4g \
--executor-memory 8g \
--conf spark.scheduler.minRegisteredResourcesRatio=1.0 \
--conf spark.scheduler.maxRegisteredResourcesWaitingTime=5m \
/home/gguzun/spark/sparkHybrid.jar gguzun/data/indexed/knn/higgs_28.rdd 25

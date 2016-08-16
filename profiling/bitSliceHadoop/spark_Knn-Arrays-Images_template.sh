#!/bin/bash

export HADOOP_OPTS=-Djava.library.path=/usr/hdp/current/hadoop-client/lib/native

/usr/hdp/2.3.4.0-3485/spark/bin/spark-submit \
--class knn.KnnArray \
--master yarn \
--deploy-mode client \
--num-executors 4 \
--executor-cores 12 \
--driver-memory 5g \
--executor-memory 12g \
--conf spark.scheduler.minRegisteredResourcesRatio=1.0 \
--conf spark.scheduler.maxRegisteredResourcesWaitingTime=5m \
/home/gguzun/spark/sparkHybrid.jar gguzun/data/indexed/knn/array/images100.rdd /home/gguzun/data/queryKnn_1K.csv gguzun/data/indexed/knn/classDir/imagesHalf.rdd 5 0 243 0 242 20

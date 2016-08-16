#!/bin/bash

dir=$( realpath $( dirname $0 ) )

############
# Defaults #
############
DEF_SPARK="/usr/hdp/2.3.4.0-3485/spark"
SPARK=${SPARK_HOME:-$DEF_SPARK}

logIP="192.168.0.200"
logPort=8282

masterUser="gguzun"
masterIP="192.168.0.100"
masterPort=22
masterSparkJar="/home/$masterUser/spark/sparkHybrid.jar"
masterHiggsBSI="/home/$masterUser/data/classDir/higgs.bsi"
masterLogDir="/home/$masterUser/expLogs"

# Create experiment directory
#date=$( date +%s.%N )
#date="1468530895.872483700"
uniqueID="$( date +%b%d )_baseline"

echo "Running experiment on $uniqueID."

expdir="$dir/experiments/$uniqueID"
echo "Creating run directory $expdir..." >&2
mkdir -p $expdir

####################
# Experiment Setup #
####################

# create a named pipe so we can switch between running local commands and remote commands
#mkfifo .remoteSession
# EDIT: Cygwin's fifo implementation is very buggy. We'll just use a regular file.
#touch $expdir/remoteSessionInput

#echo "Starting ssh session to $masterUser@$masterIP:$masterPort..." >&2
#tail -f $expdir/remoteSessionInput | ssh $masterUser@$masterIP -p $masterPort > $expdir/remoteSessionOutput & remotePID=$!

#turbostatCmd="date +%s.%N > /tmp/${date}.pgadglog && echo \" \" >> /tmp/${date}.pgadglog && sudo $turboStatPath -i 0.5 >> /tmp/${date}.pgadglog"
#testCmd="date +%s.%N > /tmp/${date}.testlog && echo \" \" >> /tmp/${date}.testlog && $testPath >> /tmp/${date}.testlog"

echo ""
#knnClass="KnnHorizontal"
#higgsNum=5
for higgsNum in 5 10 15 20 28; do
	for knnClass in KnnHorizontal KnnRange; do
	#for knnClass in KnnRange KnnHorizontal; do
		# The --name option doesn't work with this version of spark, nor does the application name configuration parameter.
		# We have to assume that we are the only one running an application.
		remoteDir="$masterLogDir/$uniqueID/$knnClass$higgsNum"
		sparkSubmit="export HADOOP_OPTS=-Djava.library.path=/usr/hdp/current/hadoop-client/lib/native; $SPARK/bin/spark-submit --class knn.$knnClass --master yarn --deploy-mode client --num-executors 8 --executor-cores 6 --driver-memory 5g --executor-memory 5g $masterSparkJar gguzun/data/indexed/knn/higgs_$higgsNum.rdd gguzun/data/HIGGS_100.csv 5 28 0 30 0 6 $masterHiggsBSI 0 1 $higgsNum 20"

		remoteCmd="mkdir -p $remoteDir; "\
"cd $remoteDir; "\
"echo \"$sparkSubmit\" > submit.command; "\
"date +%s.%N > submit.time; "\
"/bin/bash submit.command > submit.out 2> submit.err & "\
"sleep 10; "\
"appId=\$(yarn application -list 2>/dev/null | grep $masterUser | cut -f 1); "\
"echo \$appId > appId; "\
"wait; "\
"sleep 1; "\
"yarn logs -applicationId \$appId > yarnLogs.log && "\
"wget --quiet --header=\"accept-encoding: gzip\" -O sparkEvents.json.gz http://jjpowerserver0.jjcluster.net:8188/ws/v1/timeline/spark_event_v01/\$appId"
		
		localDir=$expdir/$knnClass$higgsNum
		mkdir -p $localDir
		echo "Running $knnClass$higgsNum..." >&2

		logfile=$(cygpath -w "$localDir/powerlog.log")
		# Shut down the stream in case of CTRL+C
		trap "wget -O - '$logIP:$logPort/stream?command=stop&type=csvScaled&address=$logfile' --quiet" SIGINT SIGTERM

		echo "   Starting power log..." >&2
		date +%s.%N > "$localDir/powerlog.time"
		wget -O - "http://$logIP:$logPort/stream?command=start&length=10000&type=csvScaled&fields=01234567&address=$logfile" --quiet

		echo "   Submitting application..." >&2
		ssh $masterUser@$masterIP -p $masterPort "/bin/bash -c '$remoteCmd'"
		echo "   Stopping power log..." >&2
		wget -O - "http://$logIP:$logPort/stream?command=stop&type=csvScaled&address=$logfile" --quiet

		trap "exit" SIGINT SIGTERM

		echo "   Getting output from server..." >&2
		scp -P $masterPort $masterUser@$masterIP:$remoteDir/* $localDir/

		#echo "   Resting for 20 minutes..."
		#sleep 1200
		echo "   Resting for 1 second..."
		sleep 1
		echo ""
	done
done

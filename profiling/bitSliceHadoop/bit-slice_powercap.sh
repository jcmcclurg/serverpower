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

remoteTmpDir="/tmp"

# Create experiment directory
#date=$( date +%s.%N )
#date="1468530895.872483700"
#uniqueID="$( date +%b%d )_powercap"
#uniqueID="Jul18_28_powercap"

####################
# Experiment Setup #
####################

powerCtrlUser="josiah"
powerCtrlPort=22
powerController="/home/$powerCtrlUser/serverpower/power_modulating_interfaces/rapl/power_gadget"
streamListener="/home/$powerCtrlUser/serverpower/utilities/multicast_tool/multicast_listen.py"
streamSender="$dir/../../utilities/multicast_tool/multicast_send.py"

# Send q 3x just in case the remote listeners are still running.
echo q | python $streamSender
sleep 0.25
echo q | python $streamSender
sleep 0.25
echo q | python $streamSender
sleep 0.5

echo "Starting power controllers..." >&2
for ip in 192.168.0.100 192.168.0.101 192.168.0.102 192.168.0.103 192.168.0.104; do
	ssh $powerCtrlUser@$ip -p $powerCtrlPort "/bin/bash -c 'python -u $streamListener | sudo $powerController > /tmp/powerControllerLog.out 2> /tmp/powerControllerLog.err'" &
done

echo "Uploading remote commands..." >&2
scp -P $masterPort $dir/remoteCmd.sh $dir/spark_*_template.sh $masterUser@$masterIP:$remoteTmpDir

startDate=$( date +%b%d )
for uniqueIDnum in $(seq 10); do
	uniqueID="$startDate""_powercap$uniqueIDnum"

	echo "Running experiment $uniqueID."

	expdir="$dir/experiments/$uniqueID"
	echo "Creating run directory $expdir..." >&2
	mkdir -p $expdir

	cp $dir/remoteCmd.sh $expdir/remoteCmd.sh
	cp $dir/spark_*_template.sh $expdir/

	# 22-35 in interleaved order
	for powerVal in 22 35 25 32 28; do
		ctrlVal="p$powerVal"
		echo "Sending $ctrlVal to power controllers..."

		# Send it 3x, just in case.
		echo $ctrlVal | python $streamSender
		sleep 0.25
		echo $ctrlVal | python $streamSender
		sleep 0.25
		echo $ctrlVal | python $streamSender

		mkdir -p "$expdir/exp$powerVal"

		echo "Resting for 1 seconds in case you want to quit..." >&2
		sleep 1

		for queryType in Knn TopK Aggregation; do
			for algorithm in Arrays BSI; do
				for dataset in Higgs Images Synth; do
					if [ -e "$dir/spark_$queryType-$algorithm-$dataset""_template.sh" ]; then
						echo "Resting for 1 second..." >&2
						sleep 1
						echo ""
						
						localDir="$expdir/exp$powerVal/$queryType-$algorithm-$dataset"
						mkdir -p $localDir

						remoteDir="$masterLogDir/$uniqueID/$queryType-$algorithm-$dataset"
						cmdString="/bin/bash $remoteTmpDir/remoteCmd.sh $remoteDir $masterUser $queryType $algorithm $dataset"
						echo "$cmdString" > $localDir/remoteCmdString.command

						echo "Running $queryType-$algorithm-$dataset..." >&2

						logfile=$(cygpath -w "$localDir/powerlog.log")
						# Shut down the stream in case of CTRL+C
						trap "wget -O - '$logIP:$logPort/stream?command=stop&type=csvScaled&address=$logfile' --quiet; echo q | python $streamSender; echo q | python $streamSender; exit;" SIGINT SIGTERM

						echo "   Starting power log..." >&2
						date +%s.%N > "$localDir/powerlog.time"
						wget -O - "http://$logIP:$logPort/stream?command=start&length=10000&type=csvScaled&fields=01234567&address=$logfile" --quiet

						echo "   Submitting application..." >&2
						ssh $masterUser@$masterIP -p $masterPort "$cmdString"

						#trap "exit;" SIGINT SIGTERM

						echo "   Getting output from server..." >&2
						scp -P $masterPort $masterUser@$masterIP:$remoteDir/* $localDir/
						if [ -e "$localDir/appId" ]; then
							if [ ! -e "$localDir/yarnLogs.log" ]; then
								echo "   WARNING: Retrying fetch of the yarn logs..." >&2
								ssh $masterUser@masterIP -p $masterPort "yarn logs -applicationId $(cat $localDir/appId)" > $localDir/yarnLogs.log
							fi
							if [ ! -e "$localDir/sparkEvents.json.gz" ]; then
								echo "   WARNING: Retrying fetch of the spark events..." >&2
								wget -S --quiet --header="accept-encoding: gzip" -O $localDir/sparkEvents.json.gz http://jjpowerserver0.jjcluster.net:8188/ws/v1/timeline/spark_event_v01/$(cat $localDir/appId)
							fi
						else
							echo "   ERROR: Did not get APP ID!" >&2
						fi

						echo "   Stopping power log..." >&2
						wget -O - "http://$logIP:$logPort/stream?command=stop&type=csvScaled&address=$logfile" --quiet
						trap "echo q | python $streamSender; echo q | python $streamSender; exit;" SIGINT SIGTERM

						echo ""
					else
						echo "Skipping $queryType-$algorithm-$dataset because of missing template..." >&2
					fi
				done
			done
		done
		echo "Resting for 1 second..." >&2
		sleep 1
	done
done

echo "Closing down power controllers..." >&2

# Just in case the "reset on exit" doesn't work.
echo "p50" | python $streamSender

# The first q gets sent to the power controller, and causes it to quit.
# The second one (can be anything) causes the streamListener to realize
# that it doesn't have an output stream, which causes it to quit.
sleep 1
echo "q" | python $streamSender

sleep 1
echo "q" | python $streamSender


echo "Waiting for everyone to close..." >&2
# One for good measure
sleep 1
echo "q" | python $streamSender

wait

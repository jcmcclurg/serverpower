#!/bin/bash

exps=(stress signal_insert_delays rapl powerclamp cgroups)
for i in ${exps[@]}; do
	echo "Running $i."
	./run-experiment-new.sh $i interleavedRamp > $i.out 2> $i.err
	echo "Finished with $i."
	sleep 300
done

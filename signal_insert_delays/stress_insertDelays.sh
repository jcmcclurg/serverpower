#!/bin/bash

stress -c 4 &
pid=$!
( ./triangleWave 0.1 50 & tee ) | ./insertDelays $pid
pkill -P $pid
echo "Done."

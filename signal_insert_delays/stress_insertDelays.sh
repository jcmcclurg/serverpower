#!/bin/bash

stress -c 4 & pid=$!; ./insertDelays $pid; pkill -P $pid

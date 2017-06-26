#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )

defaultPowerCap="-1"
powerCap=${1:-$defaultPowerCap}

source "$dir/functions_powerCap.sh"

if [ "$powerCap" == "-1" ]; then
	stopPowerCap
else
	startPowerCap "$powerCap"
fi

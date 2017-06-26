#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )
dir=$( readlink -fm "$dir" )

for fname in "spark-defaults.conf" "metrics.properties"; do
	if [ "$1" == "test" ]; then
		templateFile="$dir/test_$fname.template"
	else
		templateFile="$dir/$fname.template"
	fi

	sed "s|__HERE__|$dir|g" "$templateFile" > "$dir/$fname"
done

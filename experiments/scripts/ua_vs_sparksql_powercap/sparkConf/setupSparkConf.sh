#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )
dir=$( readlink -fm "$dir" )

if [ "$1" == "test" ]; then
	template="$dir/spark-defaults-test.conf.template"
else
	template="$dir/spark-defaults.conf.template"
fi

sed "s|__HERE__|$dir|g" "$template" > "$dir/spark-defaults.conf"

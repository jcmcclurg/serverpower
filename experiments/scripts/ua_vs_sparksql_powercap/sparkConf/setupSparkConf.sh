#!/bin/bash

dir=$( dirname "$BASH_SOURCE" )
dir=$( readlink -fm "$dir" )

sed "s|__HERE__|$dir|g" "$dir/spark-defaults.conf.template" > "$dir/spark-defaults.conf"

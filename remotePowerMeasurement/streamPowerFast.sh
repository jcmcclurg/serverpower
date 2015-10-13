#!/bin/bash

dir=$( dirname $0 )

echo "Streaming just the power" >&2
python -u $dir/measurementServer/multicast_listen.py -n | stdbuf -oL cut -d , -f 2

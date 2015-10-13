#!/bin/bash

dir=$( dirname $0 )

python -u $dir/measurementServer/multicast_listen.py -n

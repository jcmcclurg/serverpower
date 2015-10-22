#!/bin/bash

dir=$( dirname $0 )

$dir/start_avgPowerStream.sh
$dir/avgPower.sh | $dir/feedgnuplot/bin/feedgnuplot --line --stream 0.1 --xlen 120 --legend 0 'Average power'

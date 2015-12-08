#!/bin/bash

echo "Stopping vm at "`date +%s.%N` >&2
sudo xl shutdown ubud1
rm ~/ubud1_ip

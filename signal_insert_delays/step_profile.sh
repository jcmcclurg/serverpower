#!/bin/bash

sleep 5

d="0.9"
echo "Step to $d at " `date +%s.%N` >&2
echo $d
sleep 5

d="0.1"
echo "Step to $d at " `date +%s.%N` >&2
echo $d
sleep 5

d="0.5"
echo "Step to $d at " `date +%s.%N` >&2
echo $d
sleep 5

d="0.1"
echo "Step to $d at " `date +%s.%N` >&2
echo $d
sleep 5

echo "Quitting at " `date +%s.%N` >&2
echo "q"

#!/bin/bash

pstree -A -p $@ | sed -z 's/[^(]*(\([0-9]\+\))[^(]*/\1\n/g'

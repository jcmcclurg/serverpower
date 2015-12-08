#!/bin/bash

echo $(pstree -A -p $@ | sed 's/[^0-9]\+/ /g')

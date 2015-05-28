#!/bin/bash

echo Welcome to the hypervisor cpu limiter. >&2
echo Type the limiting values. Press CTRL+C to exit. >&2
echo The range is 0-400. See xl sched-credit -c for details >&2
while true; do
	read limitVal
	sudo xl sched-credit -d Domain-0 -c $limitVal
done

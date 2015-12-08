#!/bin/bash

freqVals=$(sudo cpufreq-info -s | sed -e "s/:[^\n,]\+, \+/,/g" | sed -e "s/:.\+//g")
read rangeMin rangeMax <<<$(sudo cpufreq-info -l)

echo "Welcome to the cpufreq cpu limiter. Please make sure you have" >&2
echo "the module loaded and acpi-cpufreq driver installed (try "
echo "intel_pstate=disable) on your kernel options". >&2
echo "The range is $freqVals. The value is the CPU frequency." >&2
echo "The voltage is controlled by the processor." >&2

trap "sudo cpupower frequency-set -g ondemand; echo 'Reset cpufreq on exit.' >&2; exit;" SIGINT SIGTERM

limitVal=$rangeMax
sudo cpupower frequency-set -g userspace
echo "Set to governor to userspace (returned $?)." >&2

while [[ "$limitVal" != "q" ]]; do
	if [ $(echo "$limitVal < $rangeMin" | bc) == 1 ]; then
		limitVal=$rangeMin
	elif [ $(echo "$limitVal > $rangeMax" | bc) == 1 ]; then
		limitVal=$rangeMax
	fi
	#sudo cpufreq-set -f $limitVal
	sudo cpupower frequency-set -f $limitVal > /dev/null
	echo "Set to $limitVal" >&2
	read limitVal
done
sudo cpupower frequency-set -g ondemand
echo 'Reset cpufreq' >&2;
echo "Done" >&2

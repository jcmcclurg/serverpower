#!/bin/bash

freqVals=$(sudo cpufreq-info -s | sed -e "s/:[^\n,]\+, \+/,/g" | sed -e "s/:.\+//g")
read rangeMin rangeMax <<<$(sudo cpufreq-info -l)

echo "Welcome to the cpufreq cpu limiter. Please make sure you have" >&2
echo "the module loaded and acpi-cpufreq driver installed (try "
echo "intel_pstate=disable) on your kernel options". >&2
echo "The range is $freqVals. The value is the CPU frequency." >&2
echo "The voltage is controlled by the processor." >&2

trap "sudo cpupower frequency-set -g ondemand; echo 'Reset cpufreq on exit.' >&2; exit;" SIGINT SIGTERM

val=$rangeMax
sudo cpupower -c all frequency-set -g userspace
echo "Set to governor to userspace (returned $?)." >&2

while [[ "$val" != "q" ]]; do
	if [ $(echo "$val < $rangeMin" | bc) == 1 ]; then
		val=$rangeMin
	elif [ $(echo "$val > $rangeMax" | bc) == 1 ]; then
		val=$rangeMax
	fi
	#sudo cpufreq-set -f $val
	sudo cpupower -c all frequency-set -f $val
	echo "Set to $val:" >&2
	sudo cpupower -c all frequency-info -w >&2
	read val
done
sudo cpupower -c all frequency-set -g ondemand
echo 'Reset cpufreq' >&2;
echo "Done" >&2

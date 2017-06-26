#!/bin/bash


if [ -z "$FUNCTIONS_POWERCAP" ]; then
	FUNCTIONS_POWERCAP=1

	#source "$( dirname "$BASH_SOURCE" )/functions_misc.sh"

	cfs_period_us="100000"
	cfs_quota_us_max=$[ $cfs_period_us*12 ]
	cfs_quota_us_min=$[ $cfs_quota_us_max/100]

	cgroupDevice="/sys/fs/cgroup/cpu/yarn"
	cgroupUser="yarn"

	msr_max=35
	msr_min=22
	setMSR="/home/josiah/serverpower/power_modulating_interfaces/rapl/setMSR"

	# Below this threshold, the power cap uses cgroups. Above and at it, the power cap uses RAPL.
	mode_switch_threshold=62

	setupPowerCap() {
		echo "Setting up power capping modules..." >&2
		sudo modprobe msr
		sudo modprobe cpuid
	}

	# Usage: startPowerCap percent
	#  percent is in the range 0 = minimum, 100 = maximum
	#
	# Returns:
	# 	returnData=	""
	#
	startPowerCap() {
		local percent="$1"
		
		if [ "$percent" -le "0" ]; then
			stopPowerCap

		elif [ "$percent" -lt "$mode_switch_threshold" ]; then
			local cfs_quota_us=$[ ( ( $cfs_quota_us_max - $cfs_quota_us_min )*$percent )/( $mode_switch_threshold ) + $cfs_quota_us_min ]

			echo "Setting power cap to $percent% (cfs_quota_us=$cfs_quota_us)..." >&2

			sudo -u "$cgroupUser" bash -c "echo '$cfs_period_us' > '$cgroupDevice/cpu.cfs_period_us'"
			sudo -u "$cgroupUser" bash -c "echo '$cfs_quota_us' > '$cgroupDevice/cpu.cfs_quota_us'"

		else
			local msr_val=$[ ( ( $msr_max - $msr_min )*( $percent - $mode_switch_threshold ) )/( 100 - $mode_switch_threshold ) + $msr_min ]

			echo "Setting power cap to $percent% (msr_val=$msr_val)..." >&2
			sudo "$setMSR" -p "$msr_val"
		fi
	}

	stopPowerCap() {
		echo "Unsetting power cap..." >&2

		sudo -u "$cgroupUser" bash -c "echo '-1' > '$cgroupDevice/cpu.cfs_quota_us'"
		sudo "$setMSR"
	}
fi

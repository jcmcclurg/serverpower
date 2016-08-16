#!/bin/bash
dir=$(dirname $0)

turboStat="$dir/turbostat_josiah"

turboStatOut="/tmp/turbostat.out"
turboStatErr="/tmp/turbostat.err"
turboStatPIDFile="/tmp/turbostat.pid"

currentCommand="none"
echo "Started turbostat runner." >&2
while read cmd; do
	if [ "x$cmd" != "x$currentCommand" ]; then
		if [ "x$cmd" == "xstart" ]; then
			echo "Starting turbostat..." >&2
			rm $turboStatOut
			sudo $turboStat -d -d -t -i 0.2 -S > $turboStatOut 2> $turboStatErr & tsPID=$!
			echo "$tsPID" > $turboStatPIDFile
			currentCommand="$cmd"
		else
			echo "Killing turbostat..." >&2
			sudo kill $tsPID
			wait
			rm $turboStatPIDFile
			echo "Turbostat killed." >&2
			currentCommand="$cmd"
		fi
	fi
	if [ "x$cmd" == "xquit" ]; then
		break;
	fi
done

echo "Quit turbostat runner." >&2

#!/bin/bash

rm ~/ubud1_ip

echo "Starting vm at "`date +%s.%N` >&2
sudo xl create /etc/xen/ubud1.cfg -q

until [ -e ~/ubud1_ip ]; do
	echo "Waiting for ~/ubud1_ip to be created..."
	sleep 1
done

echo "VM's IP is "$(cat ~/ubud1_ip)

# Directory description
This directory contains various interfaces to modulate the power consumption of the server. Right now, we are focusing on the CPU utilization.

## cpufreq
Uses the legacy ACPI driver to make frequency suggestions to the processor.

## hypervisor
Interfaces with the xen hypervisor and a virtual machine (must be installed separately: See Readme within the directory) in order to cap the CPU utilization using the sched-credit interface.

## powerclamp
Uses the intel powerclamp driver, which inserts sleep time at the kernel level.

## rapl
Uses the intel rapl driver, which uses a proprietary on-chip power limiting feature.

## signal_insert_delays
Uses external userspace SIGSTOP and SIGCONT signals to modulate the running time of selected processes and their children

## stress
Uses a combination of POSIX timers and signals to modulate the running time of a known CPU-only workload (calculating square roots). The goal is to provide a simple and adjustable compute-intensive workload.

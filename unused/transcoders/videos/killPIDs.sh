#!/bin/bash
# kill processes saved in go_transcoder_FMD_control_test4.sh
./stopRemotes.sh
sudo kill -KILL $(pgrep stress)
sudo kill -KILL $(pgrep goNoDead)
sudo kill -KILL $(pgrep goTranscoders)
sudo kill -KILL $(pgrep remotePower)
sudo kill -KILL $(pgrep calcSetpoint)
sudo kill -KILL $(pgrep avconv)
sudo kill -KILL $(pgrep power_gadget)
sudo kill -KILL $(pgrep getFreq)
sudo kill -KILL $(pgrep insertDelays)
sudo kill -KILL $(pgrep integralController)




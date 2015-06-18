#!/bin/bash
#(sudo ./powerColumn.sh & tee) | tee >(sudo python powerGraph.py) | sudo python -u powerControlLoop.py | sudo python pythonStress.py > /dev/null
#sudo ./powerColumn.sh | sudo python -u powerControlLoop.py | sudo python pythonStress.py #> /dev/null
sudo ../rapl/power_gadget -e 100 | python -u powerControlLoop.py | sudo python pythonStress.py

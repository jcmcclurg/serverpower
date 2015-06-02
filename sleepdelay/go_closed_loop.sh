#!/bin/bash
#(sudo ./powerColumn.sh & tee) | tee >(sudo python powerGraph.py) | sudo python -u powerControlLoop.py | sudo python pythonStress.py > /dev/null
(sudo ./powerColumn.sh & tee) | sudo python -u powerControlLoop.py | sudo python pythonStress.py #> /dev/null

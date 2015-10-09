# -*- coding: utf-8 -*-
"""
Created on Fri Sep 25 15:49:57 2015

@author: Josiah

See
http://docs.scipy.org/doc/scipy/reference/tutorial/optimize.html#least-square-fitting-leastsq

for more info.

"""

import numpy as np
import scipy.optimize as optim
from pylab import *
import matplotlib.pyplot as plt

def sine(time,amplitude,freq,phase):
	return amplitude*sin(2*pi*freq*time + phase)

def residuals(variables, dataToFit, time):
	amplitude, freq, phase = variables
	error = dataToFit - sine(time,amplitude,freq,phase)
	return error

def sineFit(time,dataToFit,initialAmplitude,initialFreq,initialPhase):
	initialVariables = array([initialAmplitude, initialFreq, initialPhase])
	variables, cov = optim.leastsq(residuals, initialVariables, args=(dataToFit, time ))
	amplitude, freq, phase = variables
	if(amplitude < 0):
		amplitude *= -1.0
		freq *= -1.0
		phase *= -1.0

	if(freq < 0):
		freq *= -1.0
		phase *= -1.0
		phase += pi

	phase = mod(phase,2*pi)

	return (amplitude, freq, phase)

if __name__ == '__main__':

	time = linspace(0,1/60.,256)
	actualAmplitude=  120.0*sqrt(2) + 10*randn();
	actualFreq = 60.0 + randn();
	actualPhase = rand()*2*pi;
	actual = sine(time, actualAmplitude, actualFreq, actualPhase)
	noisy =  actual + 20*randn(len(time))

	#print residuals([170,60,0], data, time)
	amplitude, freq, phase = sineFit(time,noisy,170,60,0)
	estimated = sine(time,amplitude,freq, phase)

	plt.plot(time, actual, '-', time, noisy, 'o', time, estimated, '-')
	plt.title('Least squares fit')
	plt.legend(['Actual','Noisy','Estimated'])
	plt.show()
	print "E(amplitude)=%g V, E(freq)=%g Hz, E(phase)=%g rad"%(amplitude-actualAmplitude, freq-actualFreq, phase-actualPhase)
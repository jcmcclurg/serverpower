# -*- coding: utf-8 -*-
"""
Created on Fri Sep 25 15:49:57 2015

@author: Josiah

See
http://docs.scipy.org/doc/scipy/reference/tutorial/optimize.html#least-square-fitting-leastsq

for more info.

"""

from sineFit import *

import numpy as np
import matplotlib.pyplot as plt
import scipy.interpolate as interp

def moving_average(a, n=3) :
    ret = np.cumsum(a)
    ret[n:] = ret[n:] - ret[:-n]
    return ret[n - 1:] / n

if __name__ == '__main__':
	totalDuration = 10 # total duration (in seconds) of the simulation
	maxFreqDeviation = 1  # maximum amount (in Hz) that the frequency can deviate
	freqChangePeriod = 1  # period (in seconds) for which the frequency is constant
	maxAmpDeviation = 10   # maximum amount (in Volts) that the amplitude can deviate
	ampChangePeriod = 0.1 # period (in seconds) for which the amplitude is constant
	sampleRate = 10000    # rate (in samples / second) at which the signal is sampled

	time = np.arange(0,totalDuration, 1./float(sampleRate))
	numSamples = time.size

	# Frequency deviation is a random walk
	tempTime = np.arange(0,totalDuration,float(freqChangePeriod))
	freqDeviation = np.cumsum(np.random.randn(tempTime.size))
	freqDeviation = maxFreqDeviation*freqDeviation/np.max(np.abs(freqDeviation))
	sampAndHoldFD = interp.interp1d(tempTime, freqDeviation, kind='nearest', bounds_error=False,fill_value=0)
	freqDeviation = sampAndHoldFD(time)

	# Amplitude is also a random walk
	tempTime = np.arange(0,totalDuration,float(ampChangePeriod))
	ampDeviation = np.cumsum(np.random.randn(tempTime.size))
	ampDeviation = maxAmpDeviation*ampDeviation/np.max(np.abs(ampDeviation))
	sampAndHoldFD = interp.interp1d(tempTime, ampDeviation, kind='nearest', bounds_error=False,fill_value=0)
	ampDeviation = sampAndHoldFD(time)

	initialPhase = np.random.rand()*2*pi
	phase = -1*np.concatenate(([0], np.diff(freqDeviation)))
	phase = 2*pi*phase*time
	phase = np.cumsum(phase) + initialPhase

	nominalAmplitude = 120.0*sqrt(2)
	nominalFreq = 60.0

	actualVoltage = sine(time, nominalAmplitude + ampDeviation, nominalFreq + freqDeviation, phase)
	measuredVoltage = actualVoltage + 10*randn(time.size)

	blockLen = int(np.round(10.0*sampleRate/nominalFreq))
	numBlocks = int(np.ceil(numSamples/float(blockLen)))
	offset = 0
	estimatedAmplitude = np.zeros(numSamples)
	estimatedFrequency = np.zeros(numSamples)
	estimatedPhase = np.zeros(numSamples)

	eamplitude = nominalAmplitude
	efreq = nominalFreq
	ephase = 0.0
	for i in range(0,numBlocks - 1):
		timeBlock = np.arange(0,blockLen) + offset
		eamplitude, efreq, ephase = sineFit(time[timeBlock],measuredVoltage[timeBlock],eamplitude,efreq,ephase)
		estimatedAmplitude[timeBlock] = eamplitude
		estimatedFrequency[timeBlock] = efreq
		estimatedPhase[timeBlock] = ephase
		offset += blockLen

	eamplitude, efreq, ephase = sineFit(time[offset:],measuredVoltage[offset:],eamplitude,efreq,ephase)
	estimatedAmplitude[offset:] = eamplitude
	estimatedFrequency[offset:] = efreq
	estimatedPhase[offset:] = ephase
	estimatedVoltage = sine(time,estimatedAmplitude,estimatedFrequency,estimatedPhase)

	plt.close("all")
	#plt.plot(time, actualVoltage - estimatedVoltage)
	#plt.plot(time, actualVoltage)
	#plt.plot(time, estimatedVoltage)

	avgLen = 2
	avgRange = np.arange(numSamples - (avgLen*blockLen - 1)) + (avgLen*blockLen - 1)/2
	plt.plot(time[avgRange], (nominalFreq +freqDeviation)[avgRange] - moving_average(estimatedFrequency,avgLen*blockLen))
	plt.title('Least squares fit')
	plt.legend(['Estimate Error'])
	plt.show()
	#print "E(amplitude)=%g V, E(freq)=%g Hz, E(phase)=%g rad"%(amplitude-actualAmplitude, freq-actualFreq, phase-actualPhase)
# -*- coding: utf-8 -*-
"""
Created on Sat Oct 10 12:34:28 2015

@author: Josiah
"""
from sineFit import *
import numpy as np

totalDuration = 10.0
sampleRate = 10000
time = np.arange(0,totalDuration, 1./float(sampleRate))
s = sine(time,170,59.09,0)

#n = int(np.power(2, np.ceil(np.log2(s.size))))
n = s.size

# See http://www.ingelec.uns.edu.ar/pds2803/Materiales/Articulos/AnalisisFrecuencial/04205098.pdf
#X = np.fft.rfft(s[0:n]*np.hamming(n), n)
X = np.fft.rfft(s[0:n])
freqs = np.fft.fftfreq(n, d=1./float(sampleRate))
k = np.argmax(np.abs(X))
delta = -np.real((X[k+1] - X[k-1])/(2*X[k] - X[k-1] - X[k+1]))
kpeak = k + delta
ftone = kpeak*float(sampleRate)/n

print "%g"%(ftone)
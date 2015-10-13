# -*- coding: utf-8 -*-
"""
Created on Fri Apr 25 03:04:57 2014

this is based very loosely on example code from the PyDAQmx website

@author: Josiah McClurg
"""

from NI_DAQmx import *
import StringIO
import numpy as np
import waitress
import thread
import win32api
import datetime
import cgi

from sineFit import *

class MeasurementServer(object):
	def __init__(self, task, port=8282):
		self.task = task
		self.port = port

	def webApp(self, environ, start_response):
		if environ['PATH_INFO'] == '/csv':
			defaultLen = 10;
			qs = cgi.parse_qs(environ['QUERY_STRING'])

			length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
			length = np.min([np.max([1,length]), self.task.dataWindow.size]);

			status = '200 OK'
			s = StringIO.StringIO()
			b = self.task.dataWindow.getLIFO(length);

			np.savetxt(s,b[::-1,:])

			output = s.getvalue()
			response_headers = [('Cache-Control', 'no-cache, no-store, must-revalidate'),
							('Pragma', 'no-cache'),
							('Expires', '0'),
							('Content-type', 'text/plain'),
							('Content-Length', str(len(output)))]

		else:
			status = '404 Not Found'
			output = 'Page not found.'
			response_headers = [('Content-type', 'text/plain'),
							('Content-Length', str(len(output)))]

		start_response(status, response_headers)
		return [output]

	def handler(self,dwCtrlType, hook_sigint=thread.interrupt_main):
		if dwCtrlType == 0: # CTRL_C_EVENT
			print "Stopping..."
			hook_sigint()
			return 1 # don't chain to the next handler
		return 0 # chain to the next handler

	def serve(self):
		self.task.StartTask()
		print "Started."
		try:
			win32api.SetConsoleCtrlHandler(self.handler)
			waitress.serve(self.webApp,host='0.0.0.0', port=self.port)
		except:
			pass
		finally:
			print "Stopped."
			self.task.StopTask()
			self.task.ClearTask()

if __name__ == '__main__':
	import sys

	class flushfile():
		def __init__(self, f):
			self.f = f

		def __getattr__(self,name):
			return object.__getattribute__(self.f, name)

		def write(self, x):
			self.f.write(x)
			self.f.flush()

	# Make the output unbuffered
	sys.stdout = flushfile(sys.stdout)

	class MyMeasurementServer(MeasurementServer):
		def __init__(self, port=None):
			self.voltageScalingFactor = 124.0/6.55;

			rackResistor = 0.02;
			rackGain = 10;
			self.rackScalingFactor = 1.0/(rackResistor*rackGain);

			serverResistor = 0.02;
			serverGain = 50;
			self.serverScalingFactor = 1.0/(serverResistor*serverGain);

			self.voltageIndex = 1;
			self.rackIndex = 2;
			self.serverIndices = np.array([0, 4, 3, 2, 1]) + 3;
			self.sampleRate = 10000;

			channels = []
			channels.append(AIChannelSpec('JDAQ', 0, 'voltage', termConf=DAQmx_Val_Diff, rangemin=-10, rangemax=10))
			channels.append(AIChannelSpec('JDAQ', 1, 'rack', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 2, 'server0', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 3, 'server4', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 4, 'server3', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 5, 'server2', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 6, 'server1', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))

			m = MultiChannelAITask(channels,sampleRate=self.sampleRate,dataWindowLength=int(np.round(self.sampleRate*100.0)))
			if port is None:
				super(MyMeasurementServer, self).__init__(m)
			else:
				super(MyMeasurementServer, self).__init__(m, port)

		def webApp(self, environ, start_response):
			#print "%s serving %s to %s"%(datetime.datetime.now().isoformat(), environ['PATH_INFO'], environ['REMOTE_ADDR'])
			if environ['PATH_INFO'] in ['/csvScaled', '/power', '/freq']:
				contentType = None

				if environ['PATH_INFO'] == '/csvScaled':
					defaultLen = 100
				elif environ['PATH_INFO'] == '/power':
					defaultLen = 1000
				elif environ['PATH_INFO'] == '/freq':
					defaultLen = 10000

				qs = cgi.parse_qs(environ['QUERY_STRING'])

				length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
				length = np.min([np.max([1,length]), self.task.dataWindow.size]);

				status = '200 OK'
				b = self.task.dataWindow.getLIFO(length);
				b = b[::-1,:]

				b[:,self.voltageIndex] *= self.voltageScalingFactor;
				b[:,self.rackIndex] *= self.rackScalingFactor;
				b[:,self.serverIndices] *= self.serverScalingFactor;
				b[:,3:8] = b[:,self.serverIndices]

				numSamples = b.shape[0]

				if environ['PATH_INFO'] == '/csvScaled':
					s = StringIO.StringIO()
					np.savetxt(s,b)
					output = s.getvalue()
				elif environ['PATH_INFO'] == '/power':
					defaultBlockLen = numSamples
					blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
					blockLen = np.max([1 , np.min([blockLen, numSamples])])
					numBlocks = int(np.ceil(float(numSamples)/float(blockLen)))

					b[:,self.rackIndex] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[0]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[1]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[2]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[3]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[4]] *= b[:,self.voltageIndex];

					r = np.zeros((numBlocks,8))
					offset = 0
					for i in range(0,numBlocks-1):
						blockRange = np.arange(0,blockLen) + offset
						r[i,0] = b[blockRange[0],0]
						r[i,1] = b[blockRange[-1],0]
						r[i,2:] = np.mean(b[blockRange,2:],axis=0)
						offset += blockLen

					r[-1,0] = b[offset,0]
					r[-1,1] = b[-1,0]
					r[-1,2:] = np.mean(b[offset:,2:],axis=0)

					s = StringIO.StringIO()
					np.savetxt(s,r)

					#s2 = StringIO.StringIO()
					#np.savetxt(s2,b)
					#output = s.getvalue()+"\n\n"+s2.getvalue()
					output = s.getvalue()
				elif environ['PATH_INFO'] == '/freq':
					defaultEstimatorType = "fft"
					estimatorType = str(cgi.escape(qs.get('t', [str(defaultEstimatorType)])[0]))

					if estimatorType in ['nlls', 'fft'] and length >= 10:
						nominalAmplitude = 120.0*np.sqrt(2.0)
						nominalFreq = 60.0

						sampleRate = self.task.sampleRate

						if estimatorType == 'nlls':
							defaultBlockLen = int(np.round(10.0*sampleRate/nominalFreq))
							blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
							blockLen = np.max([int(np.round(0.5*sampleRate/nominalFreq)) , np.min([blockLen, numSamples])])
						elif estimatorType == 'fft':
							defaultBlockLen = length
							blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
							blockLen = np.max([10 , np.min([blockLen, numSamples])])

						numBlocks = int(np.ceil(numSamples/float(blockLen)))
						offset = 0

						eamplitude = nominalAmplitude
						efreq = nominalFreq
						ephase = 0.0

						r = np.zeros((numBlocks,4))
						for i in range(0,numBlocks - 1):
							blockRange = np.arange(0,blockLen) + offset
							r[i,0] = b[blockRange[0],0]
							r[i,1] = b[blockRange[-1],0]

							if estimatorType == 'nlls':
								eamplitude, efreq, ephase = sineFit(b[blockRange,0],b[blockRange,self.voltageIndex],eamplitude,efreq,ephase)
								r[i,2] = eamplitude
								r[i,3] = efreq
							elif estimatorType == 'fft':
								# See http://www.ingelec.uns.edu.ar/pds2803/Materiales/Articulos/AnalisisFrecuencial/04205098.pdf
								X = np.fft.rfft(b[blockRange,self.voltageIndex])
								aX = np.abs(X)
								k = np.argmax(aX)
								delta = -np.real((X[k+1] - X[k-1])/(2*X[k] - X[k-1] - X[k+1]))
								kpeak = k + delta
								ftone = kpeak*float(sampleRate)/blockLen
								r[i,2] = 2*aX[k]/float(blockLen)
								r[i,3] = ftone

							#r[i,4] = ephase
							offset += blockLen

						r[-1,0] = b[offset,0]
						r[-1,1] = b[-1,0]
						if estimatorType == 'nlls':
							eamplitude, efreq, ephase = sineFit(b[offset:,0],b[offset:,self.voltageIndex],eamplitude,efreq,ephase)
							r[-1,2] = eamplitude
							r[-1,3] = efreq
						elif estimatorType == 'fft':
							n = numSamples - offset
							blockRange = np.arange(0,n) + offset
							X = np.fft.rfft(b[blockRange,self.voltageIndex])
							aX = np.abs(X)
							k = np.argmax(aX)
							delta = -np.real((X[k+1] - X[k-1])/(2*X[k] - X[k-1] - X[k+1]))
							kpeak = k + delta
							ftone = kpeak*float(sampleRate)/n
							r[-1,2] = 2*aX[k]/float(n)
							r[-1,3] = ftone
						#r[-1,4] = ephase

						htmlDisplay = bool(cgi.escape(qs.get('h', [str(0)])[0]))
						if htmlDisplay:
							contentType = 'text/html'
							output="""<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN"
    "http://www.w3.org/TR/html4/strict.dtd">
<html lang="en">
  <head>
    <meta http-equiv="content-type" content="text/html; charset=utf-8">
    <title>title</title>
  </head>
  <body>
  <table>
"""
							for i in range(0,numBlocks):
								output += "  <tr>"
								for j in range(0,3):
									output += "<td>%f</td>"%(r[i,j])

								if r[i,3] <= 59.980:
									bgcolor='0000ff'
									color  ='ffffff'
								elif r[i,3] <= 59.986:
									bgcolor='0055ff'
									color  ='ffffff'
								elif r[i,3] <= 59.990:
									bgcolor='00aaff'
									color  ='ffffff'
								elif r[i,3] <= 59.994:
									bgcolor='00ffff'
									color  ='000000'
								elif r[i,3] <= 59.998:
									bgcolor='00ff7f'
									color  ='000000'
								elif r[i,3] <= 60.002:
									bgcolor='00ff00'
									color  ='000000'
								elif r[i,3] <= 60.006:
									bgcolor='7fff00'
									color  ='000000'
								elif r[i,3] <= 60.010:
									bgcolor='ffff00'
									color  ='000000'
								elif r[i,3] <= 60.014:
									bgcolor='ffaa00'
									color  ='000000'
								elif r[i,3] <= 60.020:
									bgcolor='ff5500'
									color  ='ffffff'
								else:
									bgcolor='ff0000'
									color  ='ffffff'

								output += "<td style='background-color:#%s;color:#%s'>%0.4f</td></tr>\n"%(bgcolor,color, r[i,3])

							output += """
  </table>
  </body>
</html>"""
						else:
							s = StringIO.StringIO()
							np.savetxt(s,r)
							output = s.getvalue()
					else:
						output = "Valid estimator types are 'fft' or 'nlls' and length must be >= 10."

				if contentType is None:
					contentType = 'text/plain'
				response_headers = [('Cache-Control', 'no-cache, no-store, must-revalidate'),
								('Pragma', 'no-cache'),
								('Expires', '0'),
								('Content-type', contentType),
								('Content-Length', str(len(output)))]
				start_response(status, response_headers)
				return [output]
			else:
				return super(MyMeasurementServer,self).webApp(environ,start_response)

	s = MyMeasurementServer()
	s.serve()

	#dat = {"startTime": m.timeStarted, "stopTime": m.timeStopped, "data": m.dataWindow.getFIFO()}
	#dat['time'] = dat['data'][:,0] - dat['data'][0,0]
	#for i in range(0,len(channels)):
	#	dat[channels[i].name] = dat['data'][:,i+1]

	#print "Saving data to data.mat"
	#scipy.io.savemat('data.mat',dat,False,do_compression=True,oned_as='column')

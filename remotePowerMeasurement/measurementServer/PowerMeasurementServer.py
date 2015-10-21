# -*- coding: utf-8 -*-
"""
Created on Fri Apr 25 03:04:57 2014

this is based very loosely on example code from the PyDAQmx website

@author: Josiah McClurg
"""

from NI_DAQmx import *
import datetime
from Endpoint import *
from MulticastSocket import *
from MeasurementServer import *
from sineFit import *
import threading

class PowerStream(object):
	def __init__(self, socket, streamLength, streamBlockLen, streamType, streamIndices, streamingDelimiter):
		self.streamingSocket = socket
		self.streamIndices = streamIndices
		self.streamLength = streamLength
		self.streamBlockLen = streamBlockLen
		self.streamType = streamType
		self.streamingDelimiter=streamingDelimiter
		self.streamingNumberFormat='%.3f'
		self.currentUnstreamedLen = 0

	def data_updated(self,server,startTime,endTime,length):
		self.currentUnstreamedLen += length
		if(self.currentUnstreamedLen >= self.streamLength):
			# We don't check the length of b, and just assume that it's streamLength.
			# There's no good reason for this except simplicity of code, which is a good reason enough.
			self.currentUnstreamedLen -= self.streamLength
			b = server._getData(self.streamLength)
			if self.streamType == 'power':
				b = server._getPower(b, self.streamBlockLen)
			elif self.streamType == 'freq':
				b = server._getFreqFFT(b, self.streamBlockLen)
			
			s = StringIO.StringIO()
			np.savetxt(s,b[:,self.streamIndices],fmt=self.streamingNumberFormat,delimiter=self.streamingDelimiter)

			packet = Packet(s.getvalue(),self.streamingSocket.multicast_endpoint)
			self.streamingSocket.sendPacket(packet)
	
	def __str__(self):
		return "Stream(length=%d,blockLength=%d,type=%s,indices=%s,delimiter=%s)"%(self.streamLength,self.blockLength,self.streamIndices,self.streamingDelimiter)

	def __repr__(self):
		return self.__str__()

class PowerMeasurementServer(MeasurementServer):
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

		self.sockets = {}
		self.streams = {}
		self.streamsLock = threading.Lock()

		channels = []
		channels.append(AIChannelSpec('JDAQ', 0, 'voltage', termConf=DAQmx_Val_Diff, rangemin=-10, rangemax=10))
		channels.append(AIChannelSpec('JDAQ', 1, 'rack', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 2, 'server0', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 3, 'server4', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 4, 'server3', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 5, 'server2', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 6, 'server1', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))

		m = MultiChannelAITask(channels,sampleRate=self.sampleRate,dataWindowLength=int(np.round(self.sampleRate*100.0)), sampleEvery=500, data_updated_callback=self.data_updated)
		if port is None:
			super(PowerMeasurementServer, self).__init__(m)
		else:
			super(PowerMeasurementServer, self).__init__(m, port)

	def data_updated(self,startTime,endTime, length):
		# Don't allow any changes to be made to the stream list while it's updating.
		# If the blocking behavior causes problems, try using an if statement and setting
		# the parameter of acquire equal to False
		self.streamsLock.acquire()
		for stream in self.streams:
			self.streams[stream].data_updated(self,startTime,endTime,length)
		self.streamsLock.release()
	
	def _startStream(self, port, addr, uniqueid, streamLength, streamBlockLen, streamType, streamIndices, streamingDelimiter):
		output = None
		socketID = '%s:%d'%(addr, port)
		streamID = '%s:%d'%(socketID, uniqueid)
		if not (streamID in self.streams):
			if streamLength <= self.task.dataWindow.size and streamLength > 0:
				if streamBlockLen <= streamLength and streamBlockLen > 0:
					if streamType in ['power','csvScaled']:
						if np.max(streamIndices) > 7 or np.min(streamIndices) < 0:
							output = "for the specified type, fields must numbers between between 0 and 7"
					elif streamType == 'freq':
						if np.max(streamIndices) > 3 or np.min(streamIndices) < 0:
							output = "for the specified type, fields must numbers between between 0 and 3"
					else:
						output = "type must be power, freq, or csvScaled"
				else:
					output = "for the specified length, blockLength must be between %d and %d"%(1, streamLength)
			else:
				output = "length must be between %d and %d"%(1, self.task.dataWindow.size)
			
			if output is None:
				self.currentUnstreamedLen = 0
				
				if not (socketID in self.sockets):
					try:
						multicast_endpoint = Endpoint(port=port,hostname=addr)
						socket = MulticastSocket(multicast_endpoint,bind_single=True,debug=0)
						self.sockets[socketID] = socket
					except socket.error as serror:
						output = "bad address. could not create the stream"

				if output is None:
					self.streamsLock.acquire()
					self.streams[streamID] = PowerStream(self.sockets[socketID],streamLength,streamBlockLen,streamType,streamIndices,streamingDelimiter)
					self.streamsLock.release()
		else:
			output = "stream %s already streaming"%(streamID)

		return output
	
	def _stopAllStreams(self):
		self.streamsLock.acquire()
		stopped = copy.deepcopy(self.streams)
		self.streams = {}
		self.streamsLock.release()
		return stopped

	def _stopStream(self,port, addr, uniqueid):
		streamID = '%s:%d:%d'%(addr, port, uniqueid)
		ret = None
		if not (streamID in self.streams):
			ret = "stream %s does not exist."%(streamID)
		else:
			self.streamsLock.acquire()
			del self.streams[streamID]
			self.streamsLock.release()
		return ret

	def _getData(self,length):
		length = np.min([np.max([1,length]), self.task.dataWindow.size]);
		b = self.task.dataWindow.getLIFO(length);
		b = b[::-1,:]

		b[:,self.voltageIndex] *= self.voltageScalingFactor;
		b[:,self.rackIndex] *= self.rackScalingFactor;
		b[:,self.serverIndices] *= self.serverScalingFactor;
		b[:,3:8] = b[:,self.serverIndices]

		return b

	def _getPower(self, b, blockLen):
			numSamples = b.shape[0]
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

			return r
	
	def _getFreqNLLS(self, b, blockLen ):
		numSamples = b.shape[0]
		blockLen = np.max([int(np.round(0.5*sampleRate/nominalFreq)) , np.min([blockLen, numSamples])])
		numBlocks = int(np.ceil(numSamples/float(blockLen)))
		offset = 0
		nominalAmplitude = 120.0*np.sqrt(2.0)
		nominalFreq = 60.0

		eamplitude = nominalAmplitude
		efreq = nominalFreq
		ephase = 0.0

		r = np.zeros((numBlocks,4))
		for i in range(0,numBlocks - 1):
			blockRange = np.arange(0,blockLen) + offset
			r[i,0] = b[blockRange[0],0]
			r[i,1] = b[blockRange[-1],0]

			eamplitude, efreq, ephase = sineFit(b[blockRange,0],b[blockRange,self.voltageIndex],eamplitude,efreq,ephase)
			r[i,2] = eamplitude
			r[i,3] = efreq

			offset += blockLen

		r[-1,0] = b[offset,0]
		r[-1,1] = b[-1,0]
		eamplitude, efreq, ephase = sineFit(b[offset:,0],b[offset:,self.voltageIndex],eamplitude,efreq,ephase)
		r[-1,2] = eamplitude
		r[-1,3] = efreq

		return r

	def _getFreqFFT(self, b, blockLen):
		numSamples = b.shape[0]
		blockLen = np.max([10 , np.min([blockLen, numSamples])])
		numBlocks = int(np.ceil(numSamples/float(blockLen)))
		sampleRate = self.sampleRate
		offset = 0

		r = np.zeros((numBlocks,4))
		for i in range(0,numBlocks - 1):
			blockRange = np.arange(0,blockLen) + offset
			r[i,0] = b[blockRange[0],0]
			r[i,1] = b[blockRange[-1],0]

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

		return r

	def webApp(self, environ, start_response):
		#print "%s serving %s to %s"%(datetime.datetime.now().isoformat(), environ['PATH_INFO'], environ['REMOTE_ADDR'])
		if environ['PATH_INFO'] in ['/stream', '/csvScaled', '/power', '/freq']:
			# Start out with text/plain and 200 OK. Modified below if needed.
			contentType = 'text/plain'
			status = '200 OK'
			output = "Hi!"

			qs = cgi.parse_qs(environ['QUERY_STRING'])

			if environ['PATH_INFO'] == '/stream':
				command = str(cgi.escape(qs.get('command', ["-1"])[0]));
				streamLength = int(cgi.escape(qs.get('length', ["-1"])[0]));
				streamBlockLen = int(cgi.escape(qs.get('blockLength', ["-1"])[0]));
				streamType = str(cgi.escape(qs.get('type', ["-1"])[0]));
				streamIndices = str(cgi.escape(qs.get('fields', ["-1"])[0]));
				streamingDelimiter = str(cgi.escape(qs.get('delimiter', [","])[0]));
				port = int(cgi.escape(qs.get('port', ["9999"])[0]));
				addr = str(cgi.escape(qs.get('address', ["224.1.1.1"])[0]));
				uniqueid = int(cgi.escape(qs.get('uniqueid', ["0"])[0]));

				valid = False
				if command == 'info':
					if len(self.streams) > 0:
						output = "Streams:\n"
						for k in self.streams:
							output += "   %s:%s\n"%(k, self.streams[k])
					else:
						output = "No streams running."

				elif command == 'stopAll':
					l = self._stopAllStreams()
					output = "Stopped streams: %s"%(l)

				elif command == 'stop':
					errors = self._stopStream(port,addr,uniqueid)
					if errors is None:
						output = "Stream %s:%s:%d stopped."%(addr,port,uniqueid)
					else:
						output = "Error: %s"%(errors)

				elif command == 'start':
					try:
						streamIndices = np.array(list(streamIndices),dtype=int)
					except ValueError:
						streamIndices = np.array([-1])

					errors = self._startStream(port,addr,uniqueid,streamLength,streamBlockLen,streamType,streamIndices,streamingDelimiter)
					if errors is None:
						output = "Successfully started streaming fields %s %s of %d-sample chunks, taken in %d-length blocks (%s:%d, substream %d)"%(streamIndices, streamType, streamLength, streamBlockLen, addr, port, uniqueid)
					else:
						output = "Error: %s"%(errors)

				else:
					output = "Error: command must be start or stop"

			elif environ['PATH_INFO'] == '/csvScaled':
				defaultLen = 100
				length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
				b = self._getData(length)
				s = StringIO.StringIO()
				np.savetxt(s,b)
				output = s.getvalue()

			elif environ['PATH_INFO'] == '/power':
				defaultLen = 1000
				length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
				b = self._getData(length)
				defaultBlockLen = b.shape[0]
				blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
				r = self._getPower(b, blockLen)				

				s = StringIO.StringIO()
				np.savetxt(s,r)
				output = s.getvalue()

			elif environ['PATH_INFO'] == '/freq':
				defaultLen = 10000
				length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
				b = self._getData(length)
				defaultEstimatorType = "fft"
				estimatorType = str(cgi.escape(qs.get('t', [str(defaultEstimatorType)])[0]))

				if estimatorType in ['nlls', 'fft'] and length >= 10:
					sampleRate = self.task.sampleRate

					if estimatorType == 'nlls':
						defaultBlockLen = int(np.round(10.0*sampleRate/nominalFreq))
						blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
						r = self._getFreqNLLS(b, blockLen)
					elif estimatorType == 'fft':
						defaultBlockLen = length
						blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
						r = self._getFreqFFT(b, blockLen)

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
						for i in range(0,r.shape[0]):
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

			response_headers = [('Cache-Control', 'no-cache, no-store, must-revalidate'),
							('Pragma', 'no-cache'),
							('Expires', '0'),
							('Content-type', contentType),
							('Content-Length', str(len(output)))]
			start_response(status, response_headers)
			return [output]
		else:
			return super(PowerMeasurementServer,self).webApp(environ,start_response)

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

	s = PowerMeasurementServer()
	s.serve()

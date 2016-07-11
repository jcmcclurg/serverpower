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
from FreqServer import *
from sineFit import *
import threading
import cPickle as pickle
import numpy as np
import socket as skt
import gzip

class LogStream(object):
	def __init__(self, streamLength, streamIndices, logfile):
		self.streamLength = streamLength
		self.currentUnstreamedLen = 0
		self.streamIndices = streamIndices
		self.logfile = logfile

	def data_updated(self,server,startTime,endTime,length):
		self.currentUnstreamedLen += length
		if(self.currentUnstreamedLen >= self.streamLength):
			# We don't check the length of b, and just assume that it's streamLength.
			# There's no good reason for this except simplicity of code, which is a good reason enough.
			self.currentUnstreamedLen -= self.streamLength
			b = server._getData(self.streamLength)
			pickle.dump(b[:,self.streamIndices],self.logfile,-1)
			self.logfile.flush()
	
	def __str__(self):
		return "LogStream(length=%d,indices=%s,logfile=%s)"%(self.streamLength,self.streamIndices,self.logfile)

	def __repr__(self):
		return self.__str__()

class PowerStream(object):
	def __init__(self, socket, streamLength, streamBlockLen, streamType, estimatorType, streamIndices, streamingDelimiter):
		self.streamingSocket = socket
		self.streamIndices = streamIndices
		self.streamLength = streamLength
		self.streamBlockLen = streamBlockLen
		self.streamType = streamType
		self.estimatorType = estimatorType
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
				if self.estimatorType == 'fft':
					b = server._getFreqFFT(b, self.streamBlockLen)
				elif self.estimatorType == 'nlls':
					b = server._getFreqNLLS(b, self.streamBlockLen)
			
			s = StringIO.StringIO()
			np.savetxt(s,b[:,self.streamIndices],fmt=self.streamingNumberFormat,delimiter=self.streamingDelimiter)

			packet = Packet(s.getvalue(),self.streamingSocket.multicast_endpoint)
			self.streamingSocket.sendPacket(packet)
	
	def __str__(self):
		return "Stream(length=%d,blockLength=%d,type=%s,indices=%s,delimiter=%s)"%(self.streamLength,self.streamBlockLen,self.streamType,self.streamIndices,self.streamingDelimiter)

	def __repr__(self):
		return self.__str__()

class HardwareFreqStream(object):
	def __init__(self, socket, streamLength, streamBlockLen, streamIndices, streamingDelimiter):
		self.socket = socket
		self.streamLength = streamLength
		self.streamBlockLen = streamBlockLen
		self.streamingDelimiter = streamingDelimiter
		self.streamIndices = streamIndices
		self.streamingNumberFormat='%.3f'
	
	def data_updated(self,server, time, millihz):
		b = server._getFreqHardware(self.streamLength, self.streamBlockLen)
		s = StringIO.StringIO()
		np.savetxt(s,b[:,self.streamIndices],fmt=self.streamingNumberFormat,delimiter=self.streamingDelimiter)
		s.getvalue()
		p = Packet(s.getvalue(),self.socket.multicast_endpoint)
		self.socket.sendPacket(p)

	def __str__(self):
		return "HardwareFreqStream(length=%d,blockLength=%d,indices=%s,delimiter=%s)"%(self.streamLength,self.streamBlockLen,self.streamIndices,self.streamingDelimiter)

	def __repr__(self):
		return self.__str__()


class PowerMeasurementServer(MeasurementServer):
	def __init__(self, port=8282, logfile=None, logEvery=500, nocompress=False, verbose=False):
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

		self.defaultLogAddr = './power.log'
		self.defaultLogPort = -1

		self.defaultPowerAddr = '224.1.1.1'
		self.defaultPowerPort = 9999
		self.defaultFreqAddr = '224.1.1.2'
		self.defaultFreqPort = 9998
		self.bufferWindowLengthInSeconds = 100.0
		self.hardwareFreqMsPeriod = 500

		self.nocompress = nocompress
		self.verbose = verbose
		if self.verbose:
			print "Setting up frequency server..."
		self.freqServer = FreqServer(ms_period=self.hardwareFreqMsPeriod,dataWindowLength=int(np.round(1000.0*self.bufferWindowLengthInSeconds/float(self.hardwareFreqMsPeriod))),data_updated_callback=self.hardware_freq_data_updated,verbose=False)

		self.hardwareFreqStreams = {}
		self.hardwareFreqStreamsLock = threading.Lock()
		self.sockets = {}
		self.streams = {}
		if (logfile != None) and (logfile != ""):
			if self.nocompress:
				self.sockets["%s:-1"%(logfile)] = open(logfile,'wb')
			else:
				self.sockets["%s:-1"%(logfile)] = gzip.open(logfile,'wb')
			self.streams["%s:-1:0"%(logfile)] = LogStream(logEvery, np.array(range(8)), self.sockets["%s:-1"%(logfile)])

		self.streamsLock = threading.Lock()

		channels = []
		channels.append(AIChannelSpec('JDAQ', 0, 'voltage', termConf=DAQmx_Val_Diff, rangemin=-10, rangemax=10))
		channels.append(AIChannelSpec('JDAQ', 1, 'rack', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 2, 'server0', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 3, 'server4', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 4, 'server3', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 5, 'server2', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
		channels.append(AIChannelSpec('JDAQ', 6, 'server1', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))

		if self.verbose:
			print "Setting up & starting sampling task..."
		# sampleEvery=500 means that data_updated_callback is called every 500 samples = 3 cycles at 60 Hz
		m = MultiChannelAITask(channels,sampleRate=self.sampleRate,dataWindowLength=int(np.round(self.sampleRate*self.bufferWindowLengthInSeconds)), sampleEvery=500, data_updated_callback=self.power_data_updated)
		super(PowerMeasurementServer, self).__init__(m, port, verbose)

	def hardware_freq_data_updated(self, time, millihz):
		self.hardwareFreqStreamsLock.acquire()
		for stream in self.hardwareFreqStreams:
			self.hardwareFreqStreams[stream].data_updated(self,time,millihz)
		self.hardwareFreqStreamsLock.release()

	def power_data_updated(self,startTime,endTime, length):
		# Don't allow any changes to be made to the stream list while it's updating.
		# If the blocking behavior causes problems, try using an if statement and setting
		# the parameter of acquire equal to False
		self.streamsLock.acquire()
		for stream in self.streams:
			self.streams[stream].data_updated(self,startTime,endTime,length)
		self.streamsLock.release()
	
	def _startStream(self, port, addr, uniqueid, streamLength, streamBlockLen, streamType, estimatorType, streamIndices, streamingDelimiter):
		output = None
		if streamType == 'freq':
			if port == -1:
				port = self.defaultFreqPort
			if addr == "-1":
				addr = self.defaultFreqAddr
		elif streamType == 'power':
			if port == -1:
				port = self.defaultPowerPort
			if addr == "-1":
				addr = self.defaultPowerAddr
		elif streamType == 'csvScaled':
			if port == "-1":
				port = self.defaultLogPort
			if addr == '-1':
				addr = self.defaultLogAddr

		socketID = '%s:%d'%(addr, port)
		streamID = '%s:%d'%(socketID, uniqueid)

		if (not (streamID in self.streams)) and (not (streamID in self.hardwareFreqStreams)):
			if streamLength <= self.task.dataWindow.size and streamLength > 0:
				if (streamBlockLen <= streamLength and streamBlockLen > 0) or (streamType == 'csvScaled'):
					if streamType in ['power','csvScaled']:
						if np.max(streamIndices) > 7 or np.min(streamIndices) < 0:
							output = "for the specified type, fields must be between 0 and 7"

					elif streamType == 'freq':
						if estimatorType == 'hardware':
							if np.max(streamIndices) > 2 or np.min(streamIndices) < 0:
								output = "for the specified type, fields must numbers between between 0 and 2"
						elif estimatorType in ['nlls', 'fft']:
							if np.max(streamIndices) > 3 or np.min(streamIndices) < 0:
								output = "for the specified type, fields must numbers between between 0 and 3"
					else:
						output = "type must be power, freq, or csvScaled"
				else:
					output = "for the specified length, blockLength must be between %d and %d"%(1, streamLength)
			else:
				output = "length must be between %d and %d"%(1, self.task.dataWindow.size)
			
			if output is None:
				if not (socketID in self.sockets):
					try:
						if streamType == 'csvScaled':
							if self.nocompress:
								socket = open(addr,'wb')
							else:
								socket = gzip.open(addr,'wb')
						else:
							multicast_endpoint = Endpoint(port=port,hostname=addr)
							socket = MulticastSocket(multicast_endpoint,bind_single=True,debug=0)

						self.sockets[socketID] = socket
					except (skt.error, IOError) as serror:
						output = "bad address (%s) %s. could not create the stream"%(serror,socketID)

				if output is None:
					if streamType == 'freq' and estimatorType == 'hardware':
						self.hardwareFreqStreamsLock.acquire()
						self.hardwareFreqStreams[streamID] = HardwareFreqStream(self.sockets[socketID], streamLength, streamBlockLen, streamIndices, streamingDelimiter)
						self.hardwareFreqStreamsLock.release()
					else:
						self.streamsLock.acquire()
						if streamType == 'csvScaled':
							self.streams[streamID] = LogStream(streamLength, streamIndices, self.sockets[socketID]) 
						else:
							self.streams[streamID] = PowerStream(self.sockets[socketID], streamLength, streamBlockLen, streamType, estimatorType, streamIndices, streamingDelimiter)
						self.streamsLock.release()
		else:
			output = "stream %s already streaming"%(streamID)

		return output
	
	def _stopAllStreams(self):
		self.streamsLock.acquire()
		self.hardwareFreqStreamsLock.acquire()
		stopped = {}
		stopped.update(self.streams)
		stopped.update(self.hardwareFreqStreams)
		self.streams = {}
		self.hardwareFreqStreams = {}
		self.hardwareFreqStreamsLock.release()
		self.streamsLock.release()

		for s in self.sockets:
			print "Closing %s"%(self.sockets[s])
			self.sockets[s].close()
		self.sockets = {}

		return stopped

	def _stopStream(self,port, addr, uniqueid):
		streamID = '%s:%d:%d'%(addr, port, uniqueid)
		socketID = '%s:%d'%(addr, port)
		ret = None
		if streamID in self.streams:
			self.streamsLock.acquire()
			del self.streams[streamID]
			self.streamsLock.release()
		elif streamID in self.hardwareFreqStreams:
			self.hardwareFreqStreamsLock.acquire()
			del self.hardwareFreqStreams[streamID]
			self.hardwareFreqStreamsLock.release()
		else:
			ret = "stream %s does not exist."%(streamID)
		
		if ret == None:
			deleteSocket = True
			for sid in self.streams:
				if sid[0:len(socketID)] == socketID:
					deleteSocket = False
					break
			if deleteSocket:
				self.sockets[socketID].close()
				del self.sockets[socketID]
				print "Closing socket %s"%(socketID)

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

	def _getFreqHardware(self, length, blockLen):
		numSamples = np.min([np.max([1,length]), self.freqServer.dataWindow.size]);
		b = self.freqServer.dataWindow.getLIFO(numSamples);
		numSamples = b.shape[0]

		blockLen = np.max([1 , np.min([blockLen, numSamples])])
		numBlocks = int(np.ceil(float(numSamples)/float(blockLen)))
		b = b[::-1,:]
		# The raw frequency is reported in millihertz
		b[:,1] *= 0.001

		r = np.zeros((numBlocks,3))
		offset = 0
		for i in range(0,numBlocks - 1):
			blockRange = np.arange(0,blockLen) + offset
			r[i,0] = b[blockRange[0],0]
			r[i,1] = b[blockRange[-1],0]
			r[i,2] = np.mean(b[blockRange,1],axis=0)
			offset += blockLen

		r[-1,0] = b[offset,0]
		r[-1,1] = b[-1,0]
		r[-1,2] = np.mean(b[offset:,1],axis=0)
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
	
	def serve(self):
		if self.verbose:
			print "Starting frequency server..."
		self.freqServer.start(autojoin=False)

		if self.verbose:
			print "Starting measurementServer web interface..."
		r = super(PowerMeasurementServer,self).serve()
		if self.verbose:
			print "Web interface finished with exit status %s"%(r)

		if self.freqServer.running:
			if self.verbose:
				print "Stopping frequency server..."
			self.freqServer.stop()

		if self.verbose:
			print "."

	def webApp(self, environ, start_response):
		#print "%s serving %s to %s"%(datetime.datetime.now().isoformat(), environ['PATH_INFO'], environ['REMOTE_ADDR'])
		if environ['PATH_INFO'] in ['/stream', '/csvScaled', '/power', '/freq','/help']:
			# Start out with text/plain and 200 OK. Modified below if needed.
			contentType = 'text/plain'
			status = '200 OK'
			output = "Hi!"

			qs = cgi.parse_qs(environ['QUERY_STRING'])

			if environ['PATH_INFO'] == '/help':
				output = """/
 stream?
        command=[info,stopAll,stop,start]
        length
        blockLength
        type=[freq,power,csvScaled]
           For type=freq: estimator=[hardware,nlls,fft]
                          address=[multicast group]
           For type=csvScaled: address=[filename of log file]
           For type=power: address=[multicast group] 
        fields
        delimiter
        port
        uniqueid
 csvScaled?
 power?
 freq?
"""
			elif environ['PATH_INFO'] == '/stream':
				command = str(cgi.escape(qs.get('command', ["-1"])[0]));
				streamLength = int(cgi.escape(qs.get('length', ["-1"])[0]));
				streamBlockLen = int(cgi.escape(qs.get('blockLength', ["-1"])[0]));
				streamType = str(cgi.escape(qs.get('type', ["power"])[0]));
				# For type=freq
				estimatorType = str(cgi.escape(qs.get('estimator', ["hardware"])[0]))

				streamIndices = str(cgi.escape(qs.get('fields', ["-1"])[0]));
				streamingDelimiter = str(cgi.escape(qs.get('delimiter', [","])[0]));

				defaultPort = "-1"
				defaultAddr = "-1"
				if streamType == 'freq':
					defaultPort = self.defaultFreqPort
					defaultAddr = self.defaultFreqAddr
				elif streamType == 'power':
					defaultPort = self.defaultPowerPort
					defaultAddr = self.defaultPowerAddr
				elif streamType == 'csvScaled':
					defaultPort = self.defaultLogPort
					defaultAddr = self.defaultLogAddr

				port = int(cgi.escape(qs.get('port', [str(defaultPort)])[0]));
				addr = str(cgi.escape(qs.get('address', [str(defaultAddr)])[0]));
				uniqueid = int(cgi.escape(qs.get('uniqueid', ["0"])[0]));

				valid = False
				if command == 'info':
					output = ""

					self.streamsLock.acquire()
					if len(self.streams) > 0:
						output += "Streams from DAQ:\n"
						for k in self.streams:
							output += "   %s:%s\n"%(k, self.streams[k])
					self.streamsLock.release()

					self.hardwareFreqStreamsLock.acquire()
					if len(self.hardwareFreqStreams) > 0:
						output += "Streams from hardware frequency measurement:\n"
						for k in self.hardwareFreqStreams:
							output += "   %s:%s\n"%(k, self.hardwareFreqStreams[k])
					self.hardwareFreqStreamsLock.release()

					if output == "":
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

					errors = self._startStream(port,addr,uniqueid, streamLength,streamBlockLen,streamType,estimatorType,streamIndices,streamingDelimiter)
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
				estimatorType = str(cgi.escape(qs.get('t', ["hardware"])[0]))

				r = None
				if estimatorType == 'hardware':
					defaultLen = 1
					length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
					defaultBlockLen = 1
					blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
					r = self._getFreqHardware(length, blockLen)

				elif estimatorType in ['nlls', 'fft']:
					defaultLen = 10000
					length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
					if length > 10:
						b = self._getData(length)
						sampleRate = self.task.sampleRate

						if estimatorType == 'nlls':
							defaultBlockLen = int(np.round(10.0*sampleRate/nominalFreq))
							blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
							r = self._getFreqNLLS(b, blockLen)
						elif estimatorType == 'fft':
							defaultBlockLen = length
							blockLen = int(cgi.escape(qs.get('b', [str(defaultBlockLen)])[0]))
							r = self._getFreqFFT(b, blockLen)
					else:
						output = "Length must be >= 10 for 'fft' and 'nlls' estimator type."
				else:
					output = "Valid estimator types are 'hardware', 'fft', or 'nlls'."

				if not (r is None):
					htmlDisplay = bool(cgi.escape(qs.get('h', [str(0)])[0]))
					if htmlDisplay:
						contentType = 'text/html'
						output ='<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01//EN" "http://www.w3.org/TR/html4/strict.dtd">\n'
						output+='<html lang="en">\n'
						output+='	<head>\n'
						output+='	<meta http-equiv="content-type" content="text/html; charset=utf-8">\n'
						output+='		<title>Frequency estimate via %s</title>\n'%(estimatorType)
						output+='	</head>\n'
						output+='	<body>\n'
						output+='		<table>\n'

						for i in range(0,r.shape[0]):
							output+="			<tr>"
							for j in range(0,r.shape[1]-1):
								output += "<td>%f</td>"%(r[i,j])

							if r[i,-1] <= 59.980:
								bgcolor='0000ff'
								color  ='ffffff'
							elif r[i,-1] <= 59.986:
								bgcolor='0055ff'
								color  ='ffffff'
							elif r[i,-1] <= 59.990:
								bgcolor='00aaff'
								color  ='ffffff'
							elif r[i,-1] <= 59.994:
								bgcolor='00ffff'
								color  ='000000'
							elif r[i,-1] <= 59.998:
								bgcolor='00ff7f'
								color  ='000000'
							elif r[i,-1] <= 60.002:
								bgcolor='00ff00'
								color  ='000000'
							elif r[i,-1] <= 60.006:
								bgcolor='7fff00'
								color  ='000000'
							elif r[i,-1] <= 60.010:
								bgcolor='ffff00'
								color  ='000000'
							elif r[i,-1] <= 60.014:
								bgcolor='ffaa00'
								color  ='000000'
							elif r[i,-1] <= 60.020:
								bgcolor='ff5500'
								color  ='ffffff'
							else:
								bgcolor='ff0000'
								color  ='ffffff'

							output += "<td style='background-color:#%s;color:#%s'>%0.4f</td></tr>\n"%(bgcolor,color, r[i,-1])

						output+='		</table>\n'
						output+='	</body>\n'
						output+='</html>\n'
					else:
						s = StringIO.StringIO()
						np.savetxt(s,r)
						output = s.getvalue()

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
	import argparse

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

	parser = argparse.ArgumentParser(description="Measures the power consumption of our cluster")
	parser.add_argument('-p','--port',type=int,help='The webserver port to use',default=8282)
	parser.add_argument('-l','--log',type=str,help='The log file to use',default="")
	parser.add_argument('-e','--logevery',type=int,help='Write to the log every X number of samples.',default=500)
	parser.add_argument('-v', '--verbose', help='turn on verbose mode', action='store_true')
	parser.add_argument('-n', '--nocompress', help='turn off compression for logging', action='store_true')
	args = parser.parse_args()

	if args.verbose:
		print "Starting PowerMeasurementServer..."
	s = PowerMeasurementServer(port=args.port, logfile=args.log, logEvery=args.logevery, verbose=args.verbose, nocompress=args.nocompress)
	s.serve()
	s._stopAllStreams()

	print "Goodbye"

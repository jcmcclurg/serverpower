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

class MeasurementServer(object):
	def __init__(self, task):
		self.task = task

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
			waitress.serve(self.webApp,host='0.0.0.0', port=8282)
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
		def __init__(self):
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

			channels = []
			channels.append(AIChannelSpec('JDAQ', 0, 'voltage', termConf=DAQmx_Val_Diff, rangemin=-10, rangemax=10))
			channels.append(AIChannelSpec('JDAQ', 1, 'rack', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 2, 'server0', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 3, 'server4', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 4, 'server3', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 5, 'server2', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
			channels.append(AIChannelSpec('JDAQ', 6, 'server1', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))

			m = MultiChannelAITask(channels,sampleRate=10000)
			super(MyMeasurementServer, self).__init__(m)

		def webApp(self, environ, start_response):
			#print "%s serving %s to %s"%(datetime.datetime.now().isoformat(), environ['PATH_INFO'], environ['REMOTE_ADDR'])
			if environ['PATH_INFO'] in ['/csvScaled', '/power']:
				defaultLen = 10;
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

				if environ['PATH_INFO'] == '/csvScaled':
					s = StringIO.StringIO()
					np.savetxt(s,b)
					output = s.getvalue()
				else:
					b[:,self.rackIndex] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[0]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[1]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[2]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[3]] *= b[:,self.voltageIndex];
					b[:,self.serverIndices[4]] *= b[:,self.voltageIndex];
					r = np.mean(b,axis=0);
					r[0] = b[0,0];
					r[1] = b[-1,0];

					s = StringIO.StringIO()
					np.savetxt(s,r.reshape([1,8]))
					output = s.getvalue()

				response_headers = [('Cache-Control', 'no-cache, no-store, must-revalidate'),
								('Pragma', 'no-cache'),
								('Expires', '0'),
								('Content-type', 'text/plain'),
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

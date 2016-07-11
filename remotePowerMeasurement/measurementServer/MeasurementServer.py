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
import cgi
import sys

class MeasurementServer(object):
	def __init__(self, task, port=8282, verbose=False):
		self.task = task
		self.port = port
		self.verbose = verbose

	def webApp(self, environ, start_response):
		if self.verbose:
			print "webApp got %s"%(environ)

		if environ['PATH_INFO'] == '/csv':
			if self.verbose:
				print "Recognized a request for /csv..."

			defaultLen = 10;
			qs = cgi.parse_qs(environ['QUERY_STRING'])

			length = int(cgi.escape(qs.get('l', [str(defaultLen)])[0]));
			length = np.min([np.max([1,length]), self.task.dataWindow.size]);

			status = '200 OK'
			s = StringIO.StringIO()
			b = self.task.dataWindow.getLIFO(length);

			np.savetxt(s,b[::-1,:])

			output = s.getvalue()
			if self.verbose:
				print "Serving text of length %d..."%(len(output))

			response_headers = [('Cache-Control', 'no-cache, no-store, must-revalidate'),
							('Pragma', 'no-cache'),
							('Expires', '0'),
							('Content-type', 'text/plain'),
							('Content-Length', str(len(output)))]

		else:
			if self.verbose:
				print "Unrecognized request: %s"%(environ)

			status = '404 Not Found'
			output = 'Page not found.'
			response_headers = [('Content-type', 'text/plain'),
							('Content-Length', str(len(output)))]

		start_response(status, response_headers)
		return [output]

	def handler(self,dwCtrlType, hook_sigint=thread.interrupt_main):
		if dwCtrlType == 0: # CTRL_C_EVENT
			if self.verbose:
				print "Caught CTRL_C_EVENT. Calling sigint hook..."
			hook_sigint()
			return 1 # don't chain to the next handler
		if self.verbose:
			print "Caught dwCtrlType=%d. Chaining to next handler..."%(dwCtrlType)
		return 0 # chain to the next handler

	def serve(self):
		if self.verbose:
			print "Starting sampling task..."
		self.task.StartTask()

		if self.verbose:
			print "Setting up Ctrl Handler..."
		win32api.SetConsoleCtrlHandler(self.handler)

		s = None
		try:
			if self.verbose:
				print "Starting webserver on port %d"%(self.port)
			s = waitress.serve(self.webApp,host='0.0.0.0', port=self.port)
		except:
			if self.verbose:
				print "Unexpected error:", sys.exc_info()[0]
			pass
		finally:
			if self.verbose:
				print "Stopping sampling task..."
			self.task.StopTask()
			self.task.ClearTask()
		return s

if __name__ == '__main__':
	import argparse
	from NI_DAQmx import *

	parser = argparse.ArgumentParser(description="Measures things and serves them on a web interface")
	parser.add_argument('-p','--port',type=int,help='The webserver port to use',default=8282)
	parser.add_argument('-v', '--verbose', help='turn on verbose mode', action='store_true')

	args = parser.parse_args()

	channels = []
	channels.append(AIChannelSpec('JDAQ', 0, 'voltage', termConf=DAQmx_Val_Diff, rangemin=-10, rangemax=10))
	channels.append(AIChannelSpec('JDAQ', 1, 'rack', termConf=DAQmx_Val_Diff, rangemin=-5, rangemax=5))
	m = MultiChannelAITask(channels,sampleRate=10000,dataWindowLength=10000*100, sampleEvery=500, data_updated_callback=None)

	s = MeasurementServer(task=m,port=args.port,verbose=args.verbose)
	s.serve()

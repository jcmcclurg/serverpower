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


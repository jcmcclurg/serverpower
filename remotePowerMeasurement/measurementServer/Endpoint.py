# -*- coding: utf-8 -*-
"""
Created on Mon Oct 12 17:15:22 2015

@author: Josiah
"""
import socket

class Endpoint(object):
	def __init__(self,port,hostname = None):
		if (hostname == None) or (hostname == ""):
			self.hostname = socket.gethostname()
		else:
			self.hostname = hostname
		self.ip = socket.gethostbyname(self.hostname)
		self.port = port

	def __str__(self):
		return "Endpoint(hostname=%s,ip=%s,port=%s)" % (self.hostname,self.ip,self.port)
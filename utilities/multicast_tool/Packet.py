# -*- coding: utf-8 -*-
"""
Created on Mon Oct 12 17:15:22 2015

@author: Josiah
"""
import socket

class Packet(object):
	def __init__(self, data, remote_endpoint):
		self.data = data
		self.remote_endpoint = remote_endpoint

	def __str__(self):
		return "Packet(data=%s,remote_endpoint=%s)" % (self.data, self.remote_endpoint)
# -*- coding: utf-8 -*-
"""
Created on Mon Oct 12 17:16:16 2015

@author: Josiah
"""

from Packet import *
from Endpoint import *
import socket

class PacketSocket(socket.socket):
	def __init__(self, local_endpoint, autobind=True, timeout=0.5, bind_single=False, debug=0):
		super(PacketSocket, self).__init__(socket.AF_INET, socket.SOCK_DGRAM)
		self.debug = debug
		self.local_endpoint = local_endpoint
		self.recvfrom_timeout = timeout
		self.settimeout(self.recvfrom_timeout)
		self.bbind_single = bind_single
		self.autobind = autobind
		if self.debug > 0: print "Set socket timeout to %s" % (self.recvfrom_timeout)
		if self.autobind:
			self.packet_bind()

	def packet_bind(self):
		if self.bbind_single:
			self.bind_single()
		else:
			self.bind_all()

	def bind_single(self):
		self.bind((self.local_endpoint.ip, self.local_endpoint.port))
		if self.debug > 0: print "Bound to interface %s (%s) on port %s" % (self.local_endpoint.ip, self.local_endpoint.hostname, self.local_endpoint.port)

	def bind_all(self):
		self.bind(('',self.local_endpoint.port))
		if self.debug > 0: print "Bound all interfaces on %s port %s" % (self.local_endpoint.hostname, self.local_endpoint.port)

	# No idea why I can't override the default recvfrom method
	def i_recvfrom(self, bufsize, flags=0):
		data = None
		addr = None
		port = None
		while data is None:
			try:
				data, (addr, port) = super(PacketSocket, self).recvfrom(bufsize, flags)
			except socket.timeout:
				if self.debug > 100: print "i_recvfrom timed out"
				pass

		return data, (addr, port)

	def readPacket(self, maxlen=None):
		if maxlen == None:
			maxlen = 1024
		if self.debug > 0: print "%s reading a maximum of %d bytes" % (self.local_endpoint, maxlen)
		data, (addr, port) = self.i_recvfrom(maxlen)
		packet = Packet(data,Endpoint(port=port,hostname=addr))
		if self.debug > 1:
			print "%s read %s" % (self.local_endpoint, packet)
		elif self.debug > 0:
			print "%s read %d bytes from %s:%s" % (self.local_endpoint,len(data),addr,port)
		return packet

	def readPackets(self, num, maxlen=None):
		packets = []
		for i in range(0,num):
			if self.debug > 0: print "Reading packet %d of %d" % (i+1, num)
			packets.append(self.readPacket(maxlen))
		return packets

	def sendPacket(self, packet, debug=None):
		if debug is None:
			debug = self.debug

		if debug > 1:
			print "Sending %s" % packet
		elif debug > 0:
			print "Sending %d bytes to %s" % (len(packet.data),packet.remote_endpoint)

		self.sendto(packet.data, 0, (packet.remote_endpoint.ip, packet.remote_endpoint.port))
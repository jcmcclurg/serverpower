# -*- coding: utf-8 -*-
"""
Created on Mon Oct 12 17:18:13 2015

@author: Josiah
"""

from Packet import *
from Endpoint import *
from PacketSocket import *

# test

class MulticastSocket(PacketSocket):
	def __init__(self, multicast_endpoint, interface=None, bind_single=False,debug=0):
		super(MulticastSocket, self).__init__(local_endpoint=Endpoint(multicast_endpoint.port),autobind=False, bind_single=bind_single, debug=debug-1)

		self.debug = debug
		if interface is None:
			self.intf = socket.gethostbyname(socket.gethostname())
			if self.debug > 0: print "set intf = %s" % (self.intf)
		else:
			self.intf = interface
			if self.debug > 0: print "    intf = %s" % (self.intf)

		self.local_endpoint.ip = self.intf


		self.multicast_endpoint = multicast_endpoint
		try:
			self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
			self.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
		except:
			# SO_REUSEADDR should be equivalent to SO_REUSEPORT for
			# multicast UDP sockets (p 731, "TCP/IP Illustrated,
			# Volume 2"), but some BSD-derived systems require
			# SO_REUSEPORT to be specified explicity.  Also, not all
			# versions of Python have SO_REUSEPORT available.  So
			# if you're on a BSD-based system, and haven't upgraded
			# to Python 2.3 yet, you may find this library doesn't
			# work as expected.
			#
			pass

		self.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_TTL, 255)
		self.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_LOOP, 1)
		try:
			self.packet_bind()
		except:
			# Some versions of linux raise an exception even though
			# the SO_REUSE* options have been set, so ignore it
			#
			pass

		if self.debug > 0: print "Joining %s" % (self.multicast_endpoint)
		self.setsockopt(socket.SOL_IP, socket.IP_MULTICAST_IF, socket.inet_aton(self.intf) + socket.inet_aton('0.0.0.0'))
		self.setsockopt(socket.SOL_IP, socket.IP_ADD_MEMBERSHIP, socket.inet_aton(self.multicast_endpoint.ip) + socket.inet_aton('0.0.0.0'))

	def sendPacket(self, packet,debug=None):
		if (type(packet) == type("")) or (packet.remote_endpoint == None) or packet.remote_endpoint == "":
			packet = Packet(packet, self.multicast_endpoint)

		super(MulticastSocket, self).sendPacket(packet,debug)

	def close(self):
		self.setsockopt(socket.SOL_IP, socket.IP_DROP_MEMBERSHIP, socket.inet_aton(self.multicast_endpoint.ip) + socket.inet_aton('0.0.0.0'))
		super(MulticastSocket, self).close()

if __name__ == "__main__":
	import sys

	if len(sys.argv) > 1:
		slave = True
	else:
		slave = False

	group_port = 9999
	group_addr = '224.1.1.1'
	multicast_endpoint = Endpoint(port=group_port,hostname='224.1.1.1')
	debug=100
	if slave:
		print "Setting up listener..."
		s = MulticastSocket(multicast_endpoint,bind_single=False,debug=debug)
		p = s.readPacket()
		print p
	else:
		print "Setting up sender..."
		s = MulticastSocket(multicast_endpoint,bind_single=True,debug=debug)
		p = Packet("hello!",multicast_endpoint)
		s.sendPacket(p,debug=debug)

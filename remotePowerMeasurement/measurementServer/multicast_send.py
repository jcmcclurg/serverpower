#!/usr/bin/python -u
"""
Created on Mon Oct 12 17:18:13 2015

@author: Josiah
"""

import sys
import argparse
from MulticastSocket import *

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Reads from standard input and sends to a multicast address.')
	parser.add_argument('-p', '--port', type=int,	help='the port on which to listen', default=9999)
	parser.add_argument('-a', '--address', type=str, help='the address on which to listen', default='224.1.1.1')
	parser.add_argument('-n', '--nonewline', help='do not use a newline to send the packet', action='store_true')
	parser.add_argument('-s', '--size', type=bool, help='size of packets (used with -n)', default=1024)
	args = parser.parse_args()

	debug=0
	multicast_endpoint = Endpoint(port=args.port,hostname=args.address)
	sys.stderr.write("Setting up sender (%s:%d)\n"%(multicast_endpoint.hostname,multicast_endpoint.port))
	sys.stderr.write("Options are nonewline=%s size=%d\n"%(args.nonewline,args.size))	
	s = MulticastSocket(multicast_endpoint,bind_single=True,debug=debug)
	running = True
	while running:
		try:
			if args.nonewline:
				data = sys.stdin.readline()[:-1]
			else:
				data = sys.stdin.read(args.size)
			p = Packet(data,multicast_endpoint)
			s.sendPacket(p)
		except (KeyboardInterrupt, ValueError) as e:
			running = False
	sys.stderr.write("Goodbye.\n")

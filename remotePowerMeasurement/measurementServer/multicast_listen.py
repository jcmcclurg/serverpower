#!/usr/bin/python -u
"""
Created on Mon Oct 12 17:18:13 2015

@author: Josiah
"""

import sys
import time
import argparse
from MulticastSocket import *

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Listens to a multicast address and prints the packet data to standard output.')
	parser.add_argument('-p', '--port', type=int, help='the port on which to listen', default=9999)
	parser.add_argument('-a', '--address', help='the address on which to listen', default='224.1.1.1')
	parser.add_argument('-n', '--nonewline', help='do not print a newline after the packet data', action='store_true')
	parser.add_argument('-s', '--size', type=int, help='maximum size of packets', default=1024)
	parser.add_argument('-t', '--timestamp', help='print the timestamp before the data', action='store_true')
	parser.add_argument('-v', '--verbose', help='turn on verbose mode', action='store_true')
	args = parser.parse_args()

	debug=0
	multicast_endpoint = Endpoint(port=args.port,hostname=args.address)
	if args.verbose:
		sys.stderr.write("Setting up listener (%s:%d)\n"%(multicast_endpoint.hostname,multicast_endpoint.port))
		sys.stderr.write("Options are nonewline=%s size=%d\n"%(args.nonewline,args.size))
	s = MulticastSocket(multicast_endpoint,bind_single=False,debug=debug)
	running = True
	while running:
		try:
			p = s.readPacket(args.size)
			t = time.time()
			if args.verbose:
				sys.stderr.write("Received packet %s\n"%(p))
			if args.timestamp:
				sys.stdout.write("%f:"%(t))
			if args.nonewline:
				sys.stdout.write(p.data)
			else:
				sys.stdout.write(p.data+"\n")
		except (KeyboardInterrupt, ValueError, IOError) as e:
			running = False

	s.close()
	sys.stderr.write("Goodbye.\n")

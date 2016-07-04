#!/usr/bin/python -u
"""
Created on Mon Oct 12 17:18:13 2015

@author: Josiah
"""

import sys
import time
import argparse
import sched

sys.path.append('/home/josiah/research/serverpower/remotePowerMeasurement/measurementServer')
from MulticastSocket import *

if __name__ == "__main__":
	parser = argparse.ArgumentParser(description='Reads from standard input and sends to a multicast address.')
	parser.add_argument('-f', '--filename', type=str,	help='the file from which to read the setpoints', default='setpoints.txt')
	parser.add_argument('-p', '--port', type=int,	help='the port on which to listen', default=9997)
	parser.add_argument('-a', '--address', type=str, help='the address on which to listen', default='224.1.1.3')
	parser.add_argument('-n', '--nonewline', help='do not use a newline to send the packet', action='store_true')
	parser.add_argument('-s', '--size', type=int, help='size of packets (used with -n)', default=1024)
	parser.add_argument('-v', '--verbose', help='turn on verbose mode', action='store_true')
	args = parser.parse_args()

	multicast_endpoint = Endpoint(port=args.port,hostname=args.address)
	if args.verbose:
		sys.stderr.write("Setting up sender (%s:%d)\n"%(multicast_endpoint.hostname,multicast_endpoint.port))
		sys.stderr.write("Options are nonewline=%s size=%d\n"%(args.nonewline,args.size))	
	s = MulticastSocket(multicast_endpoint,bind_single=True)
	running = True

	def sendSetpoint(v):
		p = Packet("s%g"%(v),multicast_endpoint)
		t = time.time()
		s.sendPacket(p)
		sys.stdout.write("%g %f\n"%(v,t))		

	scheduler = sched.scheduler(time.time, time.sleep)
	f = open(args.filename, 'r')
	startTime = time.time() + 0.1
	for line in f:
		e = line.strip().split(' ')
		t = float(e[0])
		v = float(e[1])
		scheduler.enterabs(startTime + t, 1, sendSetpoint, (v,))
	try:
		scheduler.run()
	except KeyboardInterrupt:
		if args.verbose:
			sys.stderr.write("Closing.\n")
	finally:
		s.close()

	if args.verbose:
		sys.stderr.write("Goodbye.\n")

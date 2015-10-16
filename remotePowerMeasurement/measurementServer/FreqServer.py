import serial
import sys
import argparse
from MulticastSocket import *

def writeCommand(ser, cmd):
	ser.write(cmd)
	return ser.readline()

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Reads from standard input and sends to a multicast address.')
	parser.add_argument('-p', '--port', type=int,	help='the port on which to listen', default=9998)
	parser.add_argument('-a', '--address', type=str, help='the address on which to listen', default='224.1.1.2')
	parser.add_argument('-m', '--ms_period', type=int, help='period of reading', default=500)
	args = parser.parse_args()

	debug=0
	multicast_endpoint = Endpoint(port=args.port,hostname=args.address)
	sys.stderr.write("Setting up sender (%s:%d)\n"%(multicast_endpoint.hostname,multicast_endpoint.port))
	sys.stderr.write("Options are nonewline=%s size=%d\n"%(args.nonewline,args.size))	
	s = MulticastSocket(multicast_endpoint,bind_single=True,debug=debug)
	running = True

	ms_period = args.ms_period

	ser = serial.Serial(port='COM3',baudrate=115200, timeout=10)
	print "Resetting device to period of %d ms."%(ms_period)
	writeCommand(ser, 'S')
	writeCommand(ser, 'R')

	bt = [(ms_period >> 8) & 0x00FF, (ms_period) & 0x00FF ]
	writeCommand(ser, bt)
	ret = ser.readline()

	print "Starting."
	writeCommand(ser, 'B')
	running = True
	while running:
		try:
			ret = ser.readline()
			p = Packet("f"+ret,multicast_endpoint)
			s.sendPacket(p)
		except KeyboardInterrupt:
			running = False

	print "Stopping."
	writeCommand(ser, 'S')
	print "Goodbye."	
	ser.close()

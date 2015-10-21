import serial
import sys
import argparse
import time
import threading
from MulticastSocket import *

class FreqServer(object):
	def __init__(self, port=9998, address='224.1.1.2', ms_period=500, com='COM3', baud=115200, verbose=False, timeout=10):
		self.verbose = verbose
		self.ms_period = ms_period
		self.running = False
		self.lock = threading.Lock()

		multicast_endpoint = Endpoint(port=port,hostname=address)
		self.log("Setting up multicast socket %s:%d...\n"%(multicast_endpoint.hostname,multicast_endpoint.port))

		self.socket = MulticastSocket(multicast_endpoint,bind_single=True,debug=verbose)
		self.log("Set up multicast socket %s\n"%(self.socket))
		self.timeout = timeout
		self.interruptCheckPeriod = 1
		self.serial = serial.Serial(port=com,baudrate=baud, timeout=self.timeout)
		self.log("Set up serial port %s\n"%(self.serial))
		self.updatePeriod(ms_period)
		self.workerThread = None
	
	def serve(self):
		while self.running:
			try:
				ret = self.serial.readline()
				if (not (ret is None)) and (ret != ""):
					self.log("Read (%s)\n"%(ret))
					p = Packet("f"+ret,self.socket.multicast_endpoint)
					self.socket.sendPacket(p)
				else:
					self.log("Read timed out!\n")
			except:
				self.log("Error %s\n"%(sys.exc_info()[0]))
				self.stop()
		self.log("Thread finished\n")

	def join(self):
		while self.running:
			try:
				self.log("Checking for interrupts...\n")
				self.workerThread.join(self.interruptCheckPeriod)
			except KeyboardInterrupt:
				self.log("Got interrupt.\n")
				self.running = False
		self.log("Waiting...\n")
		self.workerThread.join()

	def stop(self):
		assert(self.running == True)
		self.running = False
		self.join()

		self.lock.acquire()
		self.writeCommand('S')
		self.workerThread = None
		self.lock.release()
	
	def start(self, autojoin=True):
		assert(self.running == False)
		self.running = True

		self.lock.acquire()
		self.writeCommand('B')
		self.workerThread = threading.Thread(target=self.serve)
		self.workerThread.start()
		self.lock.release()

		if autojoin:
			self.join()

	def updatePeriod(self, ms_period, autojoin=True):
		if self.running:
			needsRestart = True
			self.stop()
		else:
			needsRestart = False

		self.log("Resetting device to period of %d ms.\n"%(ms_period))
		self.lock.acquire()

		self.writeCommand('R')
		bt = [(ms_period >> 8) & 0x00FF, (ms_period) & 0x00FF ]
		self.writeCommand(bt)
		ret = self.serial.readline()
		self.ms_period = ms_period
		self.lock.release()

		if needsRestart:
			self.log("Restarting!\n")
			self.start(autojoin)
	
	def writeCommand(self, cmd):
		self.serial.write(cmd)
		return self.serial.readline()

	def close(self):
		self.serial.close()
		self.socket.close()
		self.log("Closed.\n")

	def log(self,string):
		if self.verbose:
			sys.stderr.write(string)

if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Reads from standard input and sends to a multicast address.')
	parser.add_argument('-p', '--port', type=int,	help='the port on which to listen', default=9998)
	parser.add_argument('-a', '--address', type=str, help='the address on which to listen', default='224.1.1.2')
	parser.add_argument('-m', '--ms_period', type=int, help='period of reading', default=500)
	parser.add_argument('-v', '--verbose', help='verbose mode', action='store_true')
	args = parser.parse_args()

	f = FreqServer(args.port,args.address,args.ms_period,verbose=args.verbose)
	f.start(autojoin=False)
	print "started"
	time.sleep(5)
	print "updating"
	f.updatePeriod(500)
	f.close()

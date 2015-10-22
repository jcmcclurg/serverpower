import serial
import sys
import argparse
import time
import threading
from RingBuffer import *
from MulticastSocket import *
import numpy as np

class FreqServer(object):
	def __init__(self, ms_period=500, dataWindowLength=None, com='COM3', baud=115200, verbose=False, timeout=10, data_updated_callback=None):
		self.verbose = verbose
		self.ms_period = ms_period
		self.running = False
		self.lock = threading.Lock()

		# The default is large enough to store ten seconds of a data
		if dataWindowLength is None:
			self.dataWindow = RingBuffer(int(np.round(1000.0*10.0/float(ms_period))),2);
		else:
			self.dataWindow = RingBuffer(int(dataWindowLength),2);

		self.timeout = timeout
		self.interruptCheckPeriod = 1
		self.serial = serial.Serial(port=com,baudrate=baud, timeout=self.timeout)
		self.log("Set up serial port %s\n"%(self.serial))
		self.updatePeriod(ms_period)
		self.workerThread = None
		self.data_updated_callback = data_updated_callback
	
	def serve(self):
		while self.running:
			try:
				ret = self.serial.readline()[:-1]
				t = time.time()
				if (not (ret is None)) and (ret != ""):
					self.log("Read (%s)\n"%(ret))
					r = int(ret)
					self.dataWindow.append(np.array([t, r]))
					if not (self.data_updated_callback is None):
						self.data_updated_callback(t,r)
				else:
					self.log("Read timed out!\n")
			except:
				self.workerThread = None
				self.stop()
				raise
		self.log("Thread finished\n")

	def join(self):
		while self.running:
			try:
				self.log("Checking for interrupts...\n")
				self.workerThread.join(self.interruptCheckPeriod)
			except KeyboardInterrupt:
				self.log("Got interrupt.\n")
				self.running = False
		if not (self.workerThread is None):
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

		self.writeCommand('S')
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
		if self.running:
			self.stop()
		self.serial.close()
		self.log("Closed.\n")

	def log(self,string):
		if self.verbose:
			sys.stderr.write(string)


if __name__ == '__main__':
	parser = argparse.ArgumentParser(description='Reads from standard input and sends to a multicast address.')
	parser.add_argument('-m', '--ms_period', type=int, help='period of reading', default=500)
	parser.add_argument('-v', '--verbose', help='verbose mode', action='store_true')
	parser.add_argument('-p', '--port', type=int,	help='the port on which to listen', default=9998)
	parser.add_argument('-a', '--address', type=str, help='the address on which to listen', default='224.1.1.2')

	args = parser.parse_args()

	class MyFreqServer(FreqServer):
		def __init__(self, ms_period, verbose, socket):
			self.socket = socket
			super(MyFreqServer, self).__init__(ms_period=ms_period, verbose=verbose,data_updated_callback=self.data_updated)

		def data_updated(self,time, millihz):
			p = Packet("f%d"%(millihz),self.socket.multicast_endpoint)
			self.socket.sendPacket(p)


	multicast_endpoint = Endpoint(port=args.port,hostname=args.address)
	print "Setting up multicast socket %s:%d...\n"%(multicast_endpoint.hostname,multicast_endpoint.port)
	socket = MulticastSocket(multicast_endpoint,bind_single=True,debug=args.verbose)
	print "Set up multicast socket %s\n"%(socket)

	f = MyFreqServer(args.ms_period,args.verbose,socket)
	f.start(autojoin=False)
	print "started"
	try:
		time.sleep(5)
		print "updating"
		f.updatePeriod(500)
	except KeyboardInterrupt:
		print "interrupted"
	finally:
		print "closing"
		f.close()

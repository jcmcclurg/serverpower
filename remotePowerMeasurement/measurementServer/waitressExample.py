import waitress

import time
import threading
import random

class DummyData(object):
	def __init__(self):
		self.lock = threading.Lock()
		self.updateData()
		self.thread = threading.Thread(target=self.mainLoop)
		self.running = False

	def updateData(self):
		self.lock.acquire()
		self.data = random.randint(0,100)
		print "Updated data to %d"%(self.data)
		self.lock.release()

	def readData(self):
		self.lock.acquire()
		d = self.data
		self.lock.release()
		return d

	def start(self):
		self.running = True
		self.thread.start()
		print "Thread started."
		waitress.serve(self.measurementServerApp,host='0.0.0.0', port=8282)

	def stop(self):
		self.running = False
		self.thread.join()
		print "Thread stopped."

	def mainLoop(self):
		while self.running:
			time.sleep(1)
			self.updateData()

	def measurementServerApp(self, environ, start_response):
		status = '200 OK'
		output = 'Hello World %d!'%( self.readData() )

		response_headers = [('Content-type', 'text/plain'),
						('Content-Length', str(len(output)))]

		start_response(status, response_headers)
		return [output]


if __name__ == '__main__':
	random.seed()

	d = DummyData()
	d.start()
	d.stop()
import numpy as np
import threading

"""
A 2D ring buffer using numpy arrays

Modified from
http://scimusing.wordpress.com/2013/10/25/ring-buffers-in-pythonnumpy/

The access functions are threadsafe except in the case of buffer overflow
"""

class RingBuffer(object):
	def __init__(self, length, width=1, dtype='d', debug=0):
		self.size = int(length)
		self.width = int(width)
		self.data = np.zeros([self.size, self.width],dtype=dtype)
		self.lock = threading.Lock()
		self.length = int(0)
		self.index = int(0)
		self.debug = debug

	# adds element x to ring buffer
	def append(self, x, debug=None):
		# Make space for writing
		self.lock.acquire()
		i = self.index;
		l = self.length
		self.index = (i + 1) % self.data.shape[0]
		self.length = np.min([self.size,l + 1])
		self.lock.release()

		# Do the writing
		self.data[i,:] = x

	# adds array x to ring buffer
	def extend(self, x, debug=0):
		if x.size > 0:
			# Make space for writing
			self.lock.acquire()
			if debug > 0 or self.debug > 0:
				print "Extending ring buffer (currently %d x %d) by %d x %d"%(self.length, self.width, x.shape[0],x.shape[1])

			i = self.index
			l = self.length
			x_index = (i + np.arange(x.shape[0],dtype='int32')) % self.size

			self.index = (x_index[-1] + 1) % self.size
			self.length = np.min([self.size,l + x.shape[0]])
			self.lock.release()

			# Do the writing
			self.data[x_index,:] = x

	# Returns the first-in-first-out data in the ring buffer
	def getFIFO(self, num=0, index=None, length=None):
		if index is None:
			self.lock.acquire()
			i = self.index
			l = self.length
			self.lock.release()
		else:
			i = index
			l = length

		idx = (i + np.arange(l) - l) % self.size
		if(num < 1):
			return self.data[idx[0:l],:]
		else:
			return self.data[idx[0:np.min([num,l])],:]

	def getLIFO(self, num=0, index=None, length=None):
		if index is None:
			self.lock.acquire()
			i = self.index
			l = self.length
			self.lock.release()
		else:
			i = index
			l = length

		idx = (i + np.arange(l) - l) % self.size
		# Reverse the indices
		idx = idx[::-1]
		if(num < 1):
			return self.data[idx[0:l],:]
		else:
			return self.data[idx[0:np.min([num,l])],:]

	def popFIFO(self, num=1):
		# Reduce the length
		self.lock.acquire()
		i = self.index
		l = self.length
		if(num < 1):
			self.length -= l
		else:
			self.length -= np.min([num,l])
		self.lock.release()

		# Actually do the reading
		d = self.getFIFO(num,i,l)
		return d

	def popLIFO(self, num=1):
		# We can't allow anyone to write until we've read out all the data.
		self.lock.acquire()
		i = self.index
		l = self.length
		d = self.getLIFO(num,i,l)
		self.length -= d.shape[0]
		self.index = (self.index - d.shape[0]) % self.size
		self.lock.release()
		return d

if __name__ == '__main__':
	r = RingBuffer(7,3)
	a = np.array([1,2,3])
	b = np.array([a-1,a-2,a-3])
	r.extend(b,debug=1)
	r.extend(b-1,debug=1)
	print r.getFIFO()
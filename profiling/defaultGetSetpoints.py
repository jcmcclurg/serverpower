import numpy
import re

def getSetpoints(filename):
	f=open(filename,'r')
	data = []
	for line in f:
		if re.search('^[0-9]+(\.[0-9]*)?,[0-9]+(\.[0-9]*)?$', line) != None:
			v = [ float(i) for i in line.strip().split(',') ]
			data.append(v)
	return numpy.array(data)

if __name__ == "__main__":
	data = getSetpoints("1450283415.294282500/server1/1450283415.294282500.testlog")
	print data

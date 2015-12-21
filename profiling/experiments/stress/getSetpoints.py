import numpy
def getSetpoints(filename):
	data = numpy.genfromtxt(filename,delimiter=',',skip_header=3,skip_footer=12)
	return data

if __name__ == "__main__":
	data = getSetpoints("1450289756.980881700/server1/1450289756.980881700.testlog")
	print data

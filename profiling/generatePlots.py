import numpy as np

import sys
import matplotlib.pyplot as plt
import statsmodels.api as sm


experiment = 'rapl'

#dates = {'stress':'1450289756.980881700','signal_insert_delays':'1450283415.294282500','powerclamp':'1450373827.544375800','rapl':'1450477549.649160300','cpufreq':'1450468081.615612300','hypervisor':'1450736746.670078800'}
dates = {'stress':'1450742429.184057100','signal_insert_delays':'1450752640.493576200','powerclamp':'1450767060.149459000','rapl':'1450821921.795694700','cpufreq':'1450791071.645437800','hypervisor':'1450802061.405142400'}
date = dates[experiment]

expDir = 'experiments/'+experiment
currentExpDir = expDir+'/'+date

#try:
#	sys.path.append(expDir)
#	from getSetpoints import getSetpoints
#except ImportError:
#	sys.path.pop()
from defaultGetSetpoints import getSetpoints


# http://stackoverflow.com/questions/11686720/is-there-a-numpy-builtin-to-reject-outliers-from-a-list
def reject_outliers(data, m = 2.):
    d = np.abs(data - np.median(data))
    mdev = np.median(d)
    s = d/mdev if mdev else 0.
    return data[s<m]

# http://www.swharden.com/blog/2010-06-20-smoothing-window-data-averaging-in-python-moving-triangle-tecnique/
def smoothTriangle(data,degree,dropVals=False):
	"""performs moving triangle smoothing with a variable degree."""
	"""note that if dropVals is False, output length will be identical
	to input length, but with copies of data at the flanking regions"""
	triangle=np.array(range(degree)+[degree]+range(degree)[::-1])+1
	smoothed=[]
	for i in range(degree,len(data)-degree*2):
		point=data[i:i+len(triangle)]*triangle
		smoothed.append(sum(point)/sum(triangle))
	if dropVals:
		return smoothed
	smoothed=[smoothed[0]]*(degree+degree/2)+smoothed
	while len(smoothed)<len(data):
		smoothed.append(smoothed[-1])
	return smoothed


serverNums = np.array([1,2,3,4])
powerData = np.genfromtxt(currentExpDir+'/powerlog.log',delimiter=' ', usecols=list(np.append([0],serverNums+2)))

setpointData = {}
#deleteme = np.array([])
for i in serverNums:
	setpoints = getSetpoints(currentExpDir+('/server%d/%s.testlog'%(i,date)))

	power = []
	for j in range(setpoints.shape[0]-1):
		segment = np.logical_and(powerData[:,0] >= setpoints[j,0], powerData[:,0] < setpoints[j+1,0])
		# power.append( powerData[ix_(segment , [0,i])] )
		#r = reject_outliers(powerData[segment, i][20:])
		s = smoothTriangle(powerData[segment,i][20:],100);
		r = reject_outliers(powerData[segment,i][20:] - s + s[0],3)
		power.append(r)
		#deleteme = np.append(deleteme,r)
		#print "Before: %d, After: %d"%(powerData[segment,i].size, r.size)

	setpointData[i] = {"setpoints": setpoints[:-1,1], "power": power }

numSetpoints = 10
indices = np.array(np.round(np.linspace(0,setpointData[1]["setpoints"].size -1,numSetpoints)),dtype=int)
data = [None]*numSetpoints*serverNums.size
for server in serverNums:
	order = setpointData[server]["setpoints"].argsort()
	j = server - 1
	for i in order[indices]:
		data[j] = setpointData[server]["power"][i]
		j += serverNums.size

labels = ["%g"%(i) for i in setpointData[1]["setpoints"][order[indices]]]

order = setpointData[1]["setpoints"][:-1].argsort()

fig = plt.figure()
ax = fig.add_subplot(111)
plot_opts = {"violin_width":1, "jitter_marker":None,"jitter_marker_size":0.5, "violin_alpha": 1, "violin_ec": '#95A5A6', "violin_fc": '#F9E79F', "violin_lw": 0.5}
sm.graphics.beanplot(data, ax=ax, jitter=True, plot_opts=plot_opts)

ax.xaxis.grid(True,which='minor')
ax.xaxis.grid(False,which='major')
ax.set_title("Power distribution across %d setpoints\nunder the %s interface."%(numSetpoints,experiment))
ax.set_xticks(np.array(range(numSetpoints))*serverNums.size + 0.5, minor=True)
ax.set_xticks(np.array(range(numSetpoints))*serverNums.size + 2.5, minor=False)
ax.set_xticklabels(labels, rotation=45)
ax.set_xlabel("Setpoint")
ax.set_ylabel("Power distribution (W)")
fig.set_size_inches(11.975, 5.425, forward=True)
fig.tight_layout()

plt.savefig(currentExpDir+'/plot.pdf')
print "Saved to "+currentExpDir+"/plot.pdf"

fig = plt.figure()
axs = [ax]
axs.append(fig.add_subplot(411))
axs.append(fig.add_subplot(412))
axs.append(fig.add_subplot(413))
axs.append(fig.add_subplot(414))

for server in serverNums:
	order = setpointData[server]["setpoints"].argsort()
	sm.graphics.beanplot([setpointData[server]["power"][i] for i in order], ax=axs[server], jitter=True, plot_opts=plot_opts)

	axs[server].set_ylabel("Server %d power (W)"%(server))
	axs[server].set_xticklabels([])
	axs[server].set_xticks(np.array(range(order.size)) + 1, minor=True)
	axs[server].set_xticks(np.array(range(order.size))[0::8] + 1, minor=False)

axs[1].set_title("Power distribution across all setpoints\nunder the %s interface."%(experiment))

order = setpointData[4]["setpoints"].argsort()
labels = ["%g"%(i) for i in setpointData[4]["setpoints"][order]]
axs[4].set_xticklabels(labels[0::8],rotation=45)
fig.set_size_inches(11.975, 13.425, forward=True)
fig.tight_layout(pad=0, w_pad=0, h_pad=0)
plt.savefig(currentExpDir+'/plot2.pdf')
print "Saved to "+currentExpDir+"/plot2.pdf"

# Get rid of the getSetpoints from the system path.
#sys.path.pop()
plt.show()

import numpy as np

import sys
import matplotlib.pyplot as plt
import statsmodels.api as sm

experiment = 'rapl'

dates = {'stress':'1450289756.980881700', 'signal_insert_delays':'1450283415.294282500', 'cpufreq':'1450468081.615612300','powerclamp':'1450373827.544375800','rapl':'1450477549.649160300'}
date = dates[experiment]

expDir = 'experiments/'+experiment
currentExpDir = expDir+'/'+date

try:
	sys.path.append(expDir)
	from getSetpoints import getSetpoints
except ImportError:
	sys.path.pop()
	from defaultGetSetpoints import getSetpoints

def reject_outliers(data, m = 2.):
    d = np.abs(data - np.median(data))
    mdev = np.median(d)
    s = d/mdev if mdev else 0.
    return data[s<m]

serverNums = np.array([1,2,3,4])
powerData = np.genfromtxt(currentExpDir+'/powerlog.log',delimiter=' ', usecols=list(np.append([0],serverNums+2)))

setpointData = {}
for i in serverNums:
	setpoints = getSetpoints(currentExpDir+('/server%d/%s.testlog'%(i,date)))

	power = []
	for j in range(setpoints.shape[0]-1):
		segment = np.logical_and(powerData[:,0] >= setpoints[j,0], powerData[:,0] < setpoints[j+1,0])
		# power.append( powerData[ix_(segment , [0,i])] )
		r = reject_outliers(powerData[segment, i][20:])
		power.append(r)
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
sm.graphics.beanplot(data, ax=ax, jitter=True, plot_opts={"violin_width":1, "jitter_marker":None,"jitter_marker_size":0.5})

ax.xaxis.grid(True,which='minor')
ax.xaxis.grid(False,which='major')
ax.set_title("Power distribution across %d setpoints\nunder the %s interface."%(numSetpoints,experiment))
ax.set_xticks(np.array(range(numSetpoints))*serverNums.size + 0.5, minor=True)
ax.set_xticks(np.array(range(numSetpoints))*serverNums.size + 2.5, minor=False)
ax.set_xticklabels(labels, rotation=45)
ax.set_xlabel("Setpoint")
ax.set_ylabel("Power distribution (W)")
fig.tight_layout()
plt.show()

plt.savefig(currentExpDir+'/plot.pdf')
print "Saved to "+currentExpDir+"/plot.pdf"


# Get rid of the getSetpoints from the system path.
sys.path.pop()
from pylab import *
import matplotlib.pyplot as plt
import matplotlib.animation as anim
import numpy as np
import sys
#import thread

class ydata(object):
    def __init__(self, y):
        self.y = y
        self.lims = (np.min(y),np.max(y))
        
    def add(self, newy):
        self.y = roll(self.y,-1)
        self.y[-1] = newy
        self.lims = (np.min([self.lims[0], newy]),np.max([self.lims[1], newy]))
        
ion()

n = 100
x = transpose(array([linspace(0,1,n)]))
y = zeros((n,1))
why = ydata(y)
#q = Queue.Queue()

close()
fig = figure()
line, = plot(x,y,'.-')

def update_line(i,ydat,line):
    if i:
        line.set_ydata(ydat.y)
        ylim(ydat.lims)
        
    return line,

def data_gen( ):
    try:
        a = " "
        while a:
            a = raw_input()
            print a
            try:
                why.add(float(a))
                yield True
                
            except ValueError:
                yield False
    
    except EOFError:
        print "input is done"
        pass
    
    except KeyboardInterrupt:
        pass
    
    except GeneratorExit:
        pass
        """
        i = False
        while not q.empty():
            try:
                why.add(q.get_nowait())
                i = True
            except Queue.Empty:
                break
        yield i
        """

an = anim.FuncAnimation(fig, func=update_line, frames=data_gen, fargs=(why, line),
    interval=1, blit=True, repeat=False)

plt.show(block=True)
#show()


print "done."

"""
def thrd(q):
    try:
        while True:
            a = float(raw_input())
            print a
            q.put(a)
    except:
        print "except"
        #pass
        
thread.start_new_thread(thrd, (q,))

# If there's input ready, do something, else do something
# else. Note timeout is zero so select won't block at all.
while sys.stdin in select.select([sys.stdin], [], [], 0)[0]:
    l = sys.stdin.readline()
    if l:
        eye.i = float(l)
    else: # an empty line means stdin has been closed
        eye.i = -1
else:
    i = 0
"""

#while True:
#    time.sleep(0.1)

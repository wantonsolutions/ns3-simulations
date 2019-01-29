import numpy as np
from pylab import *
from matplotlib.pyplot import figure
import matplotlib.pyplot as plt
import csv
import sys

def per(p, x, y):
    percent = np.percentile(y, p)
    px = []
    py = []
    for i in xrange(len(y)):
        if y[i] > percent:
            px.append(x[i])
            py.append(y[i])
    return px, py

microsecond=1000
milisecond=1000

echolatency = []
dredlatency = []


echotime = []
dredtime = []

bandwidth = []

figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

#ax = plt.gca()
#ax.get_xaxis().get_major_formatter().set_scientific(False)


sys.argv.pop(0)
print str(sys.argv)

plt.rcParams.update({'font.size': 20})

#plot the latencies of each individual measure
color=iter(cm.rainbow(np.linspace(0,1,2*(len(sys.argv) + 1))))
c=next(color)

filename1=sys.argv[0]
filename2=sys.argv[1]

with open(filename1,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        echotime.append(float(row[1]))
        echolatency.append(int(row[0])/microsecond/milisecond)
    plt.plot(echotime, echolatency, 'g-', label="echo client latency" , color=c)
    c=next(color)

e99t, e99lat = per(99, echotime, echolatency)
plt.plot(e99t, e99lat, 'g-', label="echo client 99 latency" , color=c)
c=next(color)


# calculate 99th percentile

with open(filename2,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        dredlatency.append(int(row[0])/microsecond/milisecond)
        dredtime.append(float(row[1]))
    plt.plot(dredtime, dredlatency, 'g-', label="D-Redundancy client latency" , color=c)
    c=next(color)

d99t, d99lat = per(99, dredtime, dredlatency)
plt.plot(d99t, d99lat, 'g-', label="dred client 99 latency" , color=c)
c=next(color)

#for the sake of compairison
plt.yscale("log")

plt.title("Latency of Echo Client vs Dredundancy")
plt.legend()
plt.xlabel("Seconds (S)")
plt.ylabel("Latency (us)")
plt.show()
plt.savefig("latencies.png")



import numpy as np
from pylab import *
from matplotlib.pyplot import figure
import matplotlib.pyplot as plt
import csv
import sys


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
color=iter(cm.rainbow(np.linspace(0,1,len(sys.argv) + 1)))
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

with open(filename2,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        dredtime.append(float(row[1]))
        dredlatency.append(int(row[0])/microsecond/milisecond)
    plt.plot(dredtime, dredlatency, 'g-', label="D-Redundancy client latency" , color=c)
    dredlatency = []
    c=next(color)

#for the sake of compairison
plt.yscale("log")

plt.title("Latency of Echo Client vs Dredundancy")
plt.legend()
plt.xlabel("Round Trip Count")
plt.ylabel("Latency (us)")
plt.show()
plt.savefig("latencies.png")

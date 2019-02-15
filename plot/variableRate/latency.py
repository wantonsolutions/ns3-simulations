import numpy as np
from pylab import *
from matplotlib.pyplot import figure
import matplotlib.pyplot as plt
import csv
import sys

def per(p, x, y):
    absval = np.percentile(y, p)
    px = []
    py = []
    for i in xrange(len(y)):
        if y[i] > absval:
            px.append(x[i])
            py.append(y[i])
    return px, py, absval

microsecond=1000
milisecond=1000

echolatency = []
dredlatency = []

echoOutstanding = []
dredOutstanding = []

dredLevel = []

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

dredColor='r'
echoColor='b'

# calculate 99th percentile


with open(filename1,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        echotime.append(float(row[1]))
        echolatency.append(int(row[0])/microsecond/milisecond)
        echoOutstanding.append(int(row[2]) - int(row[3]))
    plt.plot(echotime, echolatency, 'g-', label="echo client latency" , color=echoColor)
    #plt.plot(echotime, echoOutstanding, 'g-', label="D-Redundancy client Outstanding" , color='m')

e99t, e99lat, e99p = per(99, echotime, echolatency)
plt.axhline(y=e99p,linestyle='--', linewidth=4,  color=echoColor, label="echo client 99 percentile")
#plt.plot(e99t, e99lat, 'g-', label="echo client 99 latency" , color=c)
c=next(color)

with open(filename2,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        dredlatency.append(int(row[0])/microsecond/milisecond)
        dredtime.append(float(row[1]))
        dredOutstanding.append(int(row[2]) - int(row[3]))
        dredLevel.append(int(row[5]))
    plt.plot(dredtime, dredlatency, 'g-', label="D-Redundancy client latency" , color=dredColor)
    #plt.plot(dredtime, dredOutstanding, 'g-', label="D-Redundancy client Outstanding" , color='r')
    #plt.plot(dredtime, dredLevel, 'g--', label="D-Redundancy Level" , color='k')

d99t, d99lat, d99p = per(99, dredtime, dredlatency)
plt.axhline(y=d99p,linestyle='--', linewidth=4,  color=dredColor, label="d-red client 99 percentile")
#plt.plot(d99t, d99lat, 'g-', label="dred client 99 latency" , color=c)
c=next(color)

min99=0
max99=0
xposition=200
if e99p < d99p:
    min99 = e99p
    max99 = d99p
else:
    min99 = d99p
    max99 = e99p


x99 = [xposition,xposition]
y99 = [min99,max99]
plt.plot(x99,y99,linewidth=3,color='k')
plt.text(x=xposition+5,y=(math.floor((max99+min99)/2)), s=str(math.floor(max99-min99)),fontsize=20)

#for the sake of compairison
#plt.yscale("log")
#plt.xscale("log")

plt.title("Latency of Echo Client vs Dredundancy")
plt.legend()
plt.xlabel("Seconds (S)")
plt.ylabel("Latency (us)")
plt.show()

plt.clf()

plt.plot(echotime, echoOutstanding, 'g-', label="Outstanding Requests" , color='m')
plt.plot(dredtime, dredOutstanding, 'g-', label="D-Redundancy client Outstanding" , color='r')
plt.plot(dredtime, dredLevel, 'g--', label="D-Redundancy Level" , color='k')
plt.xlabel("Seconds (S)")
plt.ylabel("Absolute Number")
plt.legend()
plt.show()

#plt.savefig("latencies.png")



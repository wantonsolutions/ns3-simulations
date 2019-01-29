import numpy as np
from pylab import *
import matplotlib.pyplot as plt
import csv
import sys


microsecond=1000
milisecond=1000

time = []
bandwidth = []

from matplotlib.pyplot import figure
figure(num=None, figsize=(30, 4), dpi=80, facecolor='w', edgecolor='k')

#ax = plt.gca()
#ax.get_xaxis().get_major_formatter().set_scientific(False)


sys.argv.pop(0)
print str(sys.argv)

plt.rcParams.update({'font.size': 20})

#plot the latencies of each individual measure
color=iter(cm.rainbow(np.linspace(0,1,len(sys.argv) + 1)))
c=next(color)

for filename in sys.argv:
    with open(filename,'r') as csvfile:
        values = filename.split("-")
        currentBandwidth = int(float(values[2]) * (1/float(values[4])) * 16 / 1000)
        bandwidth.append(currentBandwidth)



averages = []
time = []
ninetynine = []

dred = []
raid = []
udp = []

with open("avgRaid.txt",'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        raid.append(int(row[0]))

with open("avgD.txt",'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        dred.append(int(row[0]))

with open("avgUDP.txt",'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        udp.append(int(row[0]))


print bandwidth
plt.plot(bandwidth, raid, 'g-', label="RAID 3", color='r')
plt.plot(bandwidth, dred, 'g-', label="Redundancy 3", color='g')
plt.plot(bandwidth, udp, 'g-', label="UDP Echo", color='b')


plt.rcParams.update({'font.size': 20})
plt.title("Average RTT latency vs Cover Bandwidth")
plt.xlabel("Cover Bandwidth MB/s")
plt.ylabel("Average Latenct (us)")
plt.legend()
plt.show()
plt.savefig("latencies.png")



plt.show()



#plt.clf()
#plt.plot(bandwidth, ninetynine, 'g-', label="99percentile", color='g')
#plt.legend()
#plt.show()
plt.savefig("AVGlatencies.png")



import numpy as np
from pylab import *
from matplotlib.pyplot import figure
import matplotlib.pyplot as plt
import csv
import sys


microsecond=1000
milisecond=1000

time = []
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

for filename in sys.argv:
    with open(filename,'r') as csvfile:
        values = filename.split("-")
        print values
        print (values[2],values[4])
        #packetsize ( 1 / rate/s * clients)
        currentBandwidth = int(float(values[2]) * (1/float(values[4])) * 16 / 1000)
        bandwidth.append(currentBandwidth)
        plots = csv.reader(csvfile, delimiter=',')
        for row in plots:
            time.append(int(row[0])/microsecond/milisecond)
        plt.plot(time, 'g-', label=str(currentBandwidth) + "MB/s", color=c)
        time = []
        c=next(color)


plt.title("Round Trip Latencies for UDP Raid R = 3")
plt.legend(ncol=3)
plt.xlabel("Round Trip Count")
plt.ylabel("Latency (us)")
plt.show()
plt.savefig("latencies.png")

import numpy as np
from pylab import *
import matplotlib.pyplot as plt
import csv

time = []
dtime = []
rtime = []


microsecond=1000
milisecond=1000

mu = 200
sigma = 25
n_bins = 50


from matplotlib.pyplot import figure
figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

sys.argv.pop(0)
print str(sys.argv)

#font = {'family' : 'normal',
#        'weight' : 'bold',
#        'size'   : 25}

#matplotlib.rc('font', **font)

#plot the latencies of each individual measure
for filename in sys.argv:
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        for row in plots:
            time.append(int(row[0])/microsecond/milisecond)
        num_bins = 20
        counts, bin_edges = np.histogram(time, bins=num_bins)
        cdf = np.cumsum(counts)
        plt.plot(bin_edges[1:], cdf, label=filename,color='g')
        time = []



plt.legend()

#counts, bin_edges = np.histogram(dtime, bins=num_bins)
#cdf = np.cumsum(counts)
#plt.plot(bin_edges[1:], cdf,label="D-Redundancy 2 PFat-Tree K=4 P=2", color='b')

#counts, bin_edges = np.histogram(rtime, bins=num_bins)
#cdf = np.cumsum(counts)
#plt.plot(bin_edges[1:], cdf, label="Raid 4 PFat-Tree K=4 P=4", color='r')

ax = plt.gca()
ax.get_xaxis().get_major_formatter().set_scientific(False)

plt.xlabel("Time (ms)")
plt.ylabel("Total Packets (Round Trip)")
plt.title("CDF of round trip latencies") 


#p = np.array([5.0,99.0])
#perc = mlab.prctile(cdf, p=p)

# Place red dots on the percentiles
#plt.plot((len(cdf)-1) * p/100., perc, 'ro')


plt.savefig("cdfs.png")


#plt.show()

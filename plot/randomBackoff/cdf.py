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
figure(num=None, figsize=(10, 10), dpi=80, facecolor='w', edgecolor='k')
plt.rcParams.update({'font.size': 20})

sys.argv.pop(0)
print str(sys.argv)

filename1=sys.argv[0]
filename2=sys.argv[1]

color=iter(cm.rainbow(np.linspace(0,1,2*(len(sys.argv) + 1))))
c=next(color)



with open(filename1,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        time.append(int(row[0])/microsecond/milisecond)
    sortedtime = np.sort(time)
    p = 1.0 *np.arange(len(time))/(len(time)-1)
    print( p )
    plt.plot(sortedtime,p, label="UDP request response",color=c,linewidth=4)
    c=next(color)
    time = []

with open(filename2,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        time.append(int(row[0])/microsecond/milisecond)
    sortedtime = np.sort(time)
    p = 1.0 *np.arange(len(time))/(len(time)-1)
    print( p )
    plt.plot(sortedtime,p, label="D-Redundancy",color=c,linewidth=4)
    c=next(color)
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
#plt.ylabel("Total Packets (Round Trip)")
plt.title("CDF of round trip latencies",fontsize=36) 


#p = np.array([5.0,99.0])
#perc = mlab.prctile(cdf, p=p)

# Place red dots on the percentiles
#plt.plot((len(cdf)-1) * p/100., perc, 'ro')

plt.xscale("log")
plt.show()
plt.savefig("cdfs.png")


#plt.show()

import numpy as np
from pylab import *
import matplotlib.pyplot as plt
from matplotlib import mlab
import csv

time = []

mu = 200
sigma = 25
n_bins = 50

with open('randomPacketSize.dat','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        time.append(int(row[0]))



num_bins = 20
counts, bin_edges = np.histogram(time, bins=num_bins)
cdf = np.cumsum(counts)
plt.plot(bin_edges[1:], cdf)

p = np.array([5.0,99.0])
perc = mlab.prctile(cdf, p=p)

# Place red dots on the percentiles
plt.plot((len(cdf)-1) * p/100., perc, 'ro')

plt.show()

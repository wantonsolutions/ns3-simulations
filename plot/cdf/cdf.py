import numpy as np
from pylab import *
import matplotlib.pyplot as plt
import csv

time = []

mu = 200
sigma = 25
n_bins = 50

with open('cdf2.dat','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        time.append(int(row[0]))



num_bins = 20
counts, bin_edges = np.histogram(time, bins=num_bins, normed=True)
cdf = np.cumsum(counts)
plt.plot(bin_edges[1:], cdf)

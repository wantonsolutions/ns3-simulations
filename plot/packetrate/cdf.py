import numpy as np
from pylab import *
import matplotlib.pyplot as plt
import csv

time = []

with open('cdf2.dat','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        time.append(int(row[0]))

plt.plot(time)
plt.show()


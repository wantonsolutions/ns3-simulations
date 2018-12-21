import numpy as np
from pylab import *
import matplotlib.pyplot as plt
import csv

font = {'family' : 'normal',
        'weight' : 'bold',
        'size'   : 25}
microsecond=1000
milisecond=1000

time = []
dtime = []
rtime = []

plt.xlabel("Round Trip Count")
plt.ylabel("Latency (us)")

from matplotlib.pyplot import figure
figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

ax = plt.gca()
ax.get_xaxis().get_major_formatter().set_scientific(False)

with open('random.dat','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        time.append(int(row[0])/microsecond/milisecond)

with open('raid.dat','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        rtime.append(int(row[0])/microsecond/milisecond)

with open('d.dat','r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    for row in plots:
        dtime.append(int(row[0])/microsecond/milisecond)

plt.plot(time, 'g-', label="Echo Protocol Fat-Tree K=2", color='g')
plt.legend()
plt.savefig("fat-tree.png")
plt.clf()
plt.plot(dtime, 'g-', label="D-Redundancy 2 PFat-Tree K=4 P=2", color='b')
plt.legend()
plt.savefig("d-ptree.png")
plt.clf()
plt.plot(dtime, 'g-', label="Raid 4 PFat-Tree K=4 P=4", color='r')
plt.legend()
plt.savefig("r-ptree.png")
plt.clf()

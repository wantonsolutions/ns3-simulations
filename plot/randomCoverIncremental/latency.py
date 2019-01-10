import numpy as np
from pylab import *
import matplotlib.pyplot as plt
import csv
import sys

sys.argv.pop(0)
print str(sys.argv)

font = {'family' : 'normal',
        'weight' : 'bold',
        'size'   : 25}
microsecond=1000
milisecond=1000

time = []

plt.xlabel("Round Trip Count")
plt.ylabel("Latency (us)")

from matplotlib.pyplot import figure
figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

ax = plt.gca()
ax.get_xaxis().get_major_formatter().set_scientific(False)


#plot the latencies of each individual measure
for filename in sys.argv:
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        for row in plots:
            time.append(int(row[0])/microsecond/milisecond)
        plt.plot(time, 'g-', label=filename,)
        time = []
plt.legend()
plt.savefig("latencies.png")

print "Finished with individual latencies"

for filename in sys.argv:
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        averages = []
        average = 0
        i = 0
        for row in plots:
            average += (int(row[0])/microsecond/milisecond)
            i = i+1
        average = average/i
        print average
        averages.append(average)

plt.plot(averages, 'g-', label="average")
plt.legend()
plt.savefig("AVGlatencies.png")



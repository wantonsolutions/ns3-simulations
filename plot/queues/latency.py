import numpy as np
from pylab import *
from matplotlib.pyplot import figure
import matplotlib.pyplot as plt
import csv
import sys


def column(data, colnum):
    col = []
    for row in data:
        col.append(row[colnum])
    return col

def toFloat(data):
    floats = []
    for d in data:
        floats.append(float(d))
    return floats

def toInt(data):
    ints = []
    for d in data:
        ints.append(float(d))
    return ints

figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

#ax = plt.gca()
#ax.get_xaxis().get_major_formatter().set_scientific(False)


sys.argv.pop(0)
print str(sys.argv)

plt.rcParams.update({'font.size': 20})

#plot the latencies of each individual measure
color=iter(cm.rainbow(np.linspace(0,1,1000)))
c=next(color)

filename1=sys.argv[0]


# calculate 99th percentile
data = []
feilds = []

with open(filename1,'r') as csvfile:
    plots = csv.reader(csvfile, delimiter=',')
    line = 0
    for row in plots:
        if line == 0:
            for identifier in row:
                feilds.append(identifier)
                #print identifier.split('.')
        else:
            for val in row:
                data[line-1].append(val)
        line = line + 1
        data.append([])
    data.pop()

db = dict()

times = column(data, 0)
time = toFloat(times)

index = 0
for f in feilds:
    dels = f.split('.')
    if len(dels) == 3:
        if not (dels[0] in db):           #Nodes
            db[dels[0]] = dict()
        if not (dels[1] in db[dels[0]]):  #Devices
            db[dels[0]][dels[1]] = dict()
        db[dels[0]][dels[1]][dels[2]] = toInt(column(data, index))
    if len(dels) == 4:
        if not (dels[0] in db):                     #Nodes
            db[dels[0]] = dict()
        if not (dels[1] in db[dels[0]]):            #Devices
            db[dels[0]][dels[1]] = dict()
        if not (dels[2] in db[dels[0]][dels[1]]):   #Queues
            db[dels[0]][dels[1]][dels[2]] = dict()
        db[dels[0]][dels[1]][dels[2]][dels[3]] = toInt(column(data, index))
    index = index + 1


devFields = ['TxBytes',
            'TxPackets',
            'TxDropBytes',
            'TxDropPackets',
            'RxBytes',
            'RxPackets'
            ]
queueFields = ['EnqueueBytes',
            'EnqueuePackets',
            'DropBytes',
            'DropPackets',
            'DequeueBytes',
            'DequeuePackets',
            'OccupancyBytes',
            'OccupancyPackets',
            'MinOccupancyBytes',
            'MinOccupancyPackets',
            'MaxOccupancyBytes',
            'MaxOccupancyPackets',
            ]

print "Graphing Queues"

#Plot Dev
for name in queueFields:
    for n in db:
        for dev in db[n]:
            for queue in db[n][dev]:
                if name in db[n][dev][queue]:
                    plt.plot(time,db[n][dev][queue][name],'g-', color = 'r',label= dev + queue + name)
    plt.title(name)
    plt.xlabel("Time Seconds")
    plt.ylabel("Data")
    plt.legend()
    plt.savefig(filename1 + "_" +name + "_"+  ".png")
    plt.figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')
    plt.rcParams.update({'font.size': 20})
    plt.clf()

print "Graphing Devices"
#Plot Dev
for name in devFields:
    for n in db:
        for dev in db[n]:
            if name in db[n][dev]:
                plt.plot(time,db[n][dev][name],'g-', color = 'r',label=name)
    plt.title(name)
    plt.xlabel("Time Seconds")
    plt.ylabel("Data")
    plt.legend()
    plt.savefig(".png")
    plt.savefig(filename1 + "_" +name + "_"+  ".png")
    figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')
    plt.rcParams.update({'font.size': 20})
    plt.clf()


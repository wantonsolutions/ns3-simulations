import numpy as np
from pylab import *
from matplotlib.pyplot import figure
import matplotlib.pyplot as plt
import csv
import sys
import pickle


## The point of this graphing program is to combine the latency graphs from
#previous plots with queue information.  ## This libary should take pairs of
#files as inputs. Each pair should include a .dat for the latency and packet
#drop. Each file should be coupled by the prefixes that it has. Each file should
#also be indexable by its suffex so seperate graphs can be generated for
#individual runs. Further comparitive graphs can easily be generated between
#paris of runs. The first task then is to parse the input and make sure that
#each file pair matches a prefix list. Eventually this prefix list could be an
#entire database, but the seperation of concerns is good to start with.


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

def checkFileTuples(filedb, suffixes):
    ##TODO compalain if the suffixes for each file prefix don't line up
    ##Implement later the graph will just crash anyways
    return

def organizeFilenames(filenames, suffixes):
    prefixes = dict()
    for f in filenames:
        sep = f.split('.')
        ext = sep.pop()
        pref = '.'.join(sep)
        if not pref in prefixes:
            prefixes[pref] = dict()
        if ext in suffixes: #probably cause a crash for indexing into a list
            prefixes[pref][ext] = f
    checkFileTuples(prefixes,suffixes)
    return prefixes


def populateDataBases(fileDB):
    fullDb = fileDB
    for prefix in fileDB:
        for suffix in fileDB[prefix]:
            if suffix == "csv":
                fullDb[prefix][suffix] = populateQueueDict(prefix + "." + suffix)
            if suffix == "dat":
                print prefix,suffix
                fullDb[prefix][suffix] = populateAppDict(prefix + "." + suffix)
    return fullDb



def populateQueueDict(filename):
    data = []
    feilds = []
    with open(filename,'r') as csvfile:
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

    index = 0
    for f in feilds:
        dels = f.split('.')
        if len(dels) == 1: ## TIME
            db[dels[0]] = toFloat(column(data,0))
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
    return db

def populateAppDict(filename):
    data = []
    data.append([])
    feilds = []
    with open(filename,'r') as csvfile:
        plots = csv.reader(csvfile, delimiter=',')
        line = 0
        for row in plots:
            for val in row:
                data[line].append(val)
            line = line + 1
            data.append([])
        data.pop()

    db = dict()
    ##EVerything is known parse with more verbosity later
    db["Latency"] = toFloat(column(data, 0))
    db["Time"] = toFloat(column(data,1))
    db["Sent"] = toInt(column(data,2))
    db["Received"] = toInt(column(data,3))
    db["Request-Index"] = toInt(column(data,4))
    db["D-Level"] = toInt(column(data,5))
    return db

def plotQueueFeild(qdb, plt, name, c):
    for n in qdb:
        if n == "Time":
            continue
        for dev in qdb[n]:
            for queue in qdb[n][dev]:
                if name in qdb[n][dev][queue]:
                    plt.plot(qdb['Time'],qdb[n][dev][queue][name],'g-', color = c,label= dev + queue + name)

def plotDev(ddb, plt,name, c):
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            ##print ddb[n][dev]
            if name in ddb[n][dev]:
                plt.plot(ddb['Time'],ddb[n][dev][name],'g-', color = c,label=name)
    return plt


sys.argv.pop(0)

makeCache = True
#plot the latencies of each individual measure
print sys.argv
filedict = dict()
if makeCache:    
    suffixes = ["dat","csv"]
    filenames=sys.argv
    filedict = organizeFilenames(filenames, suffixes)
    fullDB = populateDataBases(filedict)
    pickle.dump (fullDB, open("./dbcache.p","w+"))


print "Graphing Queues"

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

appFields = ['Latency',
             'Time',
             'Sent',
             'Received',
             'Request-Index',
             'D-Level',
             ]

color=iter(cm.rainbow(np.linspace(0,1,1000)))
c=next(color)
plt.rcParams.update({'font.size': 20})

#Plot Dev

for name in filedict:
            #plotDev(filedict[name][suf],plt,'TxDropPackets','r')
            fig, ax1 = plt.subplots()
            #fig(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

            ax2 = ax1.twinx()
            ax2 = plotDev(filedict[name]['csv'],ax2,'TxPackets','r')
            ax2.set_ylabel('Enqueued Packets')

            ax1.set_ylabel('Latency (ns)')
            plt.xlabel("Time (S)")
            
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            ax1.plot(time,lat,'g-',color = 'b',label=name + "--latency")


           

            plt.legend()
            plt.xlabel("Time (Seconds)")
            plt.title(name)
            plt.show()
            plt.clf()
            #figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')




fig.tight_layout()

plt.title(name)
plt.xlabel("Time Seconds")
plt.ylabel("Data")
plt.legend()
#plt.savefig(filename1 + "_" +name + "_"+  ".png")
plt.show()
figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')
plt.rcParams.update({'font.size': 20})
plt.clf()

'''
print "Graphing Devices"
#Plot Dev
for name in devFields:
    plt.title(name)
    plt.xlabel("Time Seconds")
    plt.ylabel("Data")
    plt.legend()
    plt.savefig(".png")
    plt.savefig(filename1 + "_" +name + "_"+  ".png")
    figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')
    plt.rcParams.update({'font.size': 20})
    plt.clf()

    plt.title(name)
    plt.xlabel("Time Seconds")
    plt.ylabel("Data")
    plt.legend()
    plt.savefig(filename1 + "_" +name + "_"+  ".png")
    plt.figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')
    plt.rcParams.update({'font.size': 20})
    plt.clf()
'''

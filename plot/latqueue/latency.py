import numpy as np
from pylab import *
from matplotlib.pyplot import figure
from matplotlib.font_manager import FontProperties
import matplotlib.pyplot as plt
import matplotlib.colors as colors
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
        ints.append(int(d))
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
                #print prefix,suffix
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
    db["Id"] = toInt(column(data,6))
    return db

def sumDevFeild(ddb, name):
    sumlist = []
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            ##print ddb[n][dev]
            index = 0
            if name in ddb[n][dev]:
                if len(sumlist) == 0:
                    #print(name)
                    sumlist = ddb[n][dev][name]
                else:
                    for val in ddb[n][dev][name]:
                        sumlist[index] = sumlist[index] + val
                        index = index + 1
    return sumlist


def avgDevFeild(ddb, name):
    sumf = sumDevFeild(ddb, name)
    total = 0
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            if name in ddb[n][dev]:
                total = total + 1
        break
    #print total
    avg = sumf
    index = 0
    for val in avg:
        avg[index] = float(val) / float(total)
        index += 1

    return avg

def sumQueueFeild(ddb, name):
    sumlist = []
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            for queue in ddb[n][dev]:
                index = 0
                if name in ddb[n][dev][queue]:
                    if len(sumlist) == 0:
                        #print(name)
                        sumlist = ddb[n][dev][queue][name]
                    else:
                        for val in ddb[n][dev][queue][name]:
                            sumlist[index] = sumlist[index] + val
                            index = index + 1
    return sumlist

def avgQueueFeild(ddb, name):
    sumf = sumQueueFeild(ddb, name)
    total = 0
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            for queue in ddb[n][dev]:
                if name in ddb[n][dev][queue]:
                    total = total + 1
        break
    #print total
    avg = sumf
    index = 0
    for val in avg:
        avg[index] = float(val) / float(total)
        index += 1

    return avg

globalcount = 0

def plotQueueFeild(qdb, plt, name, c, fname):

    colorp=iter(cm.rainbow(np.linspace(0,1,40)))
    cp=next(color)
    mlat = SecondsToMicro(qdb['Time'])
    #print(mlat)
    
    tmpcolor = ['b','r','c','k', 'm', 'g']
    cindex=0
    for n in qdb:
        if n == "Time":
            continue
        for dev in qdb[n]:
            for queue in qdb[n][dev]:
                if name in qdb[n][dev][queue]:
                    zeroval = True
                    for v in qdb[n][dev][queue][name]:
                        if v != 0.0:
                            zeroval = False
                    if not zeroval:
                        #plt.plot(mlat,qdb[n][dev][queue][name],'g-', color = tmpcolor[cindex%len(tmpcolor)],label= dev + queue + name)
                        plt.plot(mlat,qdb[n][dev][queue][name],'g-', color=cp,label= dev + queue + name + str(cindex) + fname, linewidth = 2)
                        #print(cp)
                        #print(dev + " " + queue + " " + name)
                        cp=next(colorp)
                        cindex += 1

                        

def plotDev(ddb, plt,name, c):
    for n in ddb:
        if n == "Time":
            continue
        for dev in ddb[n]:
            ##print ddb[n][dev]
            if name in ddb[n][dev]:
                plt.plot(ddb['Time'],ddb[n][dev][name],'g:', color=c,label=name)
    return plt

def cdf(time, data):
    high = max(data)
    low = min(data)
    norm = colors.Normalize(low,high)
    ndata = norm(data)
    H, X1 = np.histogram(ndata, bins = 100 )
    dx = X1[1] - X1[0]
    F1 = np.cumsum(H) * dx
    return X1[1:], F1
    #sortedtime = np.sort(time)
    #pdata = 1.0 *np.arange(len(time))/(len(time)-1)
    #return sortedtime, pdata



def calculateTotalSentRequests(time,sent,rec,nid):
    sentAgg = dict()
    i = 0
    diff = []
    while i < len(time):
        sentAgg[str(nid[i])] = sent[i]
        s = 0
        for r in sentAgg:
            s += sentAgg[r]
        diff.append(s)
        i += 1
    return diff


def calculateFailedRequests(time,sent,rec,nid):
    outstandingAgg = dict()
    i = 0
    diff = []
    while i < len(time):
        outstandingAgg[str(nid[i])] = sent[i] - rec[i]
        #outstandingAgg[str(nid[i])] = i
        s = 0
        for outstanding in outstandingAgg:
            s += outstandingAgg[outstanding]
        diff.append(s)
        i += 1
    return diff

def NanoToMicroSeconds(lat):
    i = 0
    for val in lat:
        lat[i] = val/1000
        i = i+1
    #print lat
    return lat

def SecondsToMicro(lat):
    i = 0
    for val in lat:
        lat[i] = val*1000*1000
        i = i+1
    #print lat
    return lat


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
             'Id',
             ]

color=iter(cm.rainbow(np.linspace(0,1,100)))
c=next(color)
plt.rcParams.update({'font.size': 20})

#Plot Dev Individual
'''
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
'''

#Plot Dev All

tmpcolor = ['b','r','c','k', 'm', 'g']

plt.xlabel("Time (S)")
plt.title("D-Redundancy v.s UDP-Echo latency")
plt.ylabel('Latency 100s of seconds')
#fig, ax1 = plt.subplots()
cindex =0
#figure(num=None, figsize=(30, 10), dpi=80, facecolor='w', edgecolor='k')

for name in filedict:
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]
            
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            sent = filedict[name]['dat']['Sent']
            rec = filedict[name]['dat']['Received']
            nid = filedict[name]['dat']['Id']


            

            sentAgg = calculateTotalSentRequests(time,sent,rec,nid)
            outstandingAgg = calculateFailedRequests(time,sent,rec,nid)

            #plt.plot(time,sentAgg,'g--',color=tmpcolor[cindex],label=finalname + " Sent-Agg")
            #cindex = cindex + 1
            #plt.plot(time,outstandingAgg,'g--',color=tmpcolor[cindex],label=finalname + " Outstanding-Agg")
            #cindex = cindex + 1
            plt.plot(time,lat,'g--',color=tmpcolor[cindex],label=finalname + " latency")
            cindex = cindex + 1
            c=next(color)
plt.legend()

#plt.show()


#rightaxisplot1='TxDropPackets'
txp='TxPackets'
rxp='RxPackets'
dpt='TxDropPackets'

#rightaxisplot3='EnqueuePackets'
eqp='EnqueuePackets'
dp='DropPackets'


#plt.title("D-Redundancy To UDP Convergance - Packet Loss and Network Fill")
plt.title('Singe Packet Trace')
plt.ylabel('Enqueued Packets (4096 Byte)')
plt.xlabel("Time us")


#ax2 = ax1.twinx()
#ax2.set_ylabel(rightaxisplot1)g,
cindex = 0
for name in filedict:
            #s = sumDevFeild(filedict[name]['csv'], 'TxPackets')
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]

            #Tx Packets 
            #s = sumDevFeild(filedict[name]['csv'], txp)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g-', color=tmpcolor[cindex],label=txp + " " + finalname)
            #c=next(color)

            #s = avgDevFeild(filedict[name]['csv'], rxp)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g-', color=c,label=rxp + " " + finalname)
            #c=next(color)

            #s = sumDevFeild(filedict[name]['csv'], dpt)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g-*', color=tmpcolor[cindex],label=dpt + " " + finalname)
            #cindex = cindex + 1

            #Enqueue packets
            s = sumQueueFeild(filedict[name]['csv'], eqp)
            plt.plot(filedict[name]['csv']['Time'], s, 'g--', color=tmpcolor[cindex],label=eqp + " " + finalname)
            c=next(color)
            cindex = cindex + 1

            #DroppedPackets
            #s = sumQueueFeild(filedict[name]['csv'], dp)
            #plt.plot(filedict[name]['csv']['Time'], s, 'g--', color128.16, 109.33, 114.532, 116.144, 104.128, 104.128, 104.128, 104.128, 104.128, 104.128, 108.534, 118.938, 104.128, 104.128, 104.128, 104.128, 104.128, 10=tmpcolor[cindex],label=dp + " " + finalname)
            #c=next(color)
            #cindex = cindex + 1


            #plotQueueFeild(filedict[name]['csv'],plt,eqp,tmpcolor[cindex],name)

            #s = sumDevFeild(filedict[name]['csv'], rightaxisplot1)
            #ax2.plot(filedict[name]['csv']['Time'], s, 'g-', color=c,label=rightaxisplot1 + " " + finalname)
            #c=next(color)

            #s = sumDevFeild(filedict[name]['csv'], rightaxisplot2)
            #ax2.plot(filedict[name]['csv']['Time'], s, 'g-', color=c,label=rightaxisplot2 + " " + finalname)
            #c=next(color)

fontP = FontProperties()
fontP.set_size('medium')
plt.legend(loc='upper right', ncol = 4, prop=fontP)
plt.legend()
#plt.show()
plt.clf()
plt.rcParams.update({'font.size': 20})

figure(num=None, figsize=(15, 15), dpi=80, facecolor='w', edgecolor='k')
cindex = 0
maxlat = 0
minlat = 999999

for name in filedict:
            
            sname = name.split("/")
            subname = sname[len(sname)-1]
            leftname = subname.split("_")
            finalname = leftname[0]
            
            lat = filedict[name]['dat']['Latency']
            time = filedict[name]['dat']['Time']
            mslat = NanoToMicroSeconds(lat)
            #ptime, plat = cdf(time,lat)README
            maxlatT = max(mslat)
            minlatT = min(mslat)
            if maxlatT > maxlat:
                maxlat = maxlatT
            if minlatT < minlat:
                minlat = minlatT
            plt.hist(mslat,1000, normed=1, histtype='step', cumulative=True, label=finalname, linewidth=3.5)
            #plt.plot(ptime,plat,'g--',color=tmpcolor[cindex],label=finalname)
            cindex = cindex + 1

#Plot max and min veritical lines, this is for the sake of demonstration

#plt.axvline(x=104, label="Theoretical Min = 104us", linestyle='--', color = 'b', linewidth = 1.5)
#plt.axvline(x=minlat, label="Observed  Min = " + str(minlat) +"us", linestyle='--',color = 'r', linewidth = 1.5)
#plt.axvline(x=208, label="Theoretical Max = 208us", linestyle='--', color = 'b', linewidth = 1.5)
#plt.axvline(x=maxlat, label="Observed  Max = " + str(maxlat) + "us",linestyle = '--', color = 'r', linewidth = 1.5)
plt.grid('on')

plt.xlabel("Time us", fontweight='bold')
#plt.ylabel("Percentile")
#plt.xscale("log")

#plt.xlim(right=250)
lgd = plt.legend(ncol=3,loc="lower center",bbox_to_anchor=(0.50,-0.20))
#plt.ylabel("Total Packets (Round Trip)")
plt.title("Round Trip Time 100% Bandwidth Saturation K=32 P=3",fontweight='bold')
plt.tight_layout(rect=(0,0.1,1,1))
plt.savefig("DvUDP.png")


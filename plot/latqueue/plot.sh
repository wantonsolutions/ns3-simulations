#!/bin/bash

dataDir="/home/ssgrant/workspace/ns-allinone-3.29/ns-3.29/data/queuelat"
#python latency.py multichannel-probe-test.csv
#python latency.py mctest.csv
#python latency.py *.csv *.dat
#python latency.py *997.dat *997.csv
#python latency.py *995.dat *995.csv
#python latency.py $dataDir/*999.dat $dataDir/*999.csv
#python latency.py $dataDir/echo_latest.dat $dataDir/echo_latest.csv
python latency.py $dataDir/*latest.dat $dataDir/*latest.csv

#python latency.py *991.dat *991.csv
#python latency.py exp_debug*

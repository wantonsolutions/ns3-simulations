#!/bin/bash

dataDir="/home/ssgrant/workspace/ns-allinone-3.29/ns-3.29/data/queuelat/"
#python latency.py multichannel-probe-test.csv
#python latency.py mctest.csv
python latency.py *.csv *.dat

#!/bin/bash

#args=`ls -v exp*` 
#python latency.py $args

#args=`ls -v exp*` 

datadir=../../data/backoff
#file1="echo-current.dat"
#file2="dred-current.dat"

file1="echo_latest.dat"
file2="dred_latest.dat"
python latency.py $datadir/$file1 $datadir/$file2
python cdf.py $datadir/$file1 $datadir/$file2


#args=`ls -v exp*` 
#python cdf.py $args

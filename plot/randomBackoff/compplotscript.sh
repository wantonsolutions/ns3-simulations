#!/bin/bash

#args=`ls -v exp*` 
#python latency.py $args

#args=`ls -v exp*` 

datadir=../../data/variableRate
#file1="echo-current.dat"
#file2="dred-current.dat"

file1="echo_latest.dat"
file2="dred_latest.dat"

dredArray=($(ls $datadir/exp_dred*))
echoArray=($(ls $datadir/exp_echo*))

for i in "${!echoArray[@]}"; do
	python latency.py ${echoArray[$i]} ${dredArray[$i]}
	python cdf.py ${echoArray[$i]} ${dredArray[$i]}
done
exit 0


#args=`ls -v exp*` 
#python cdf.py $args

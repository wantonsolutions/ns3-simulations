#!/bin/bash

#args=`ls -v exp*` 
#python latency.py $args

args=`ls -v exp*` 
python average.py $args


#args=`ls -v exp*` 
#python cdf.py $args

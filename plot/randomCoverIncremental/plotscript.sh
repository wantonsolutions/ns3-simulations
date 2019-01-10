#!/bin/bash

args=`ls -v exp*` 
python latency.py $args

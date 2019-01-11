#!/bin/bash

echo "Hello Experiment"


function runExperiment () {
	cpnp=$1
	cpi=$2
	cpps=$3
	cnp=$4
	ci=$5
	cps=$6
	filename=exp_$7_$1-$2-$3-$4-$5-$6.dat

./waf --run \
	"scratch/pfattree
	--ClientProtocolNPackets=$cpnp
	--ClientProtocolInterval=$cpi
	--ClientProtocolPacketSize=$cpps
	--CoverNPackets=$cnp
	--CoverInterval=$ci
	--CoverPacketSize=$cps
	" 2>$filename 
}

if [ $1 == "debug" ];then
	echo "debugging"
	runExperiment 100 0.1 128 100 0.1 128 "debug"
	exit 0
fi

intervals=(0.1 0.01 0.001 0.0001 0.00001 0.000001 0.0000001)
ClientProtocolNPackets=4096
ClientProtocolInterval=0.01
ClientProtocolPacketSize=1024
CoverNPackets=1000
CoverInterval=$intervals
CoverPacketSize=1024

#let 'exp=0'
#for i in ${intervals[@]}; do
#	runExperiment $ClientProtocolNPackets $ClientProtocolInterval $ClientProtocolPacketSize $CoverNPackets $i $CoverPacketSize $exp &
#	let 'exp=exp+1'
#done

let 'exp=0'
for i in `seq  0.02 -0.001 0.001`; do
	runExperiment $ClientProtocolNPackets $ClientProtocolInterval $ClientProtocolPacketSize $CoverNPackets $i $CoverPacketSize $exp &
	let 'exp=exp+1'
done



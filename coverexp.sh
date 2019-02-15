#!/bin/bash -x

echo "Hello Experiment"

debug=true

function runExperiment () {
	cpnp=$1
	cpi=$2
	cpps=$3
	cnp=$4
	ci=$5
	cps=$6

	mode=$8
	incRate=$9
	filename=exp_$7_$1-$2-$3-$4-$5-$6-$8-$9

./waf --run \
	"scratch/pfattree
	--ClientProtocolNPackets=$cpnp
	--ClientProtocolInterval=$cpi
	--ClientProtocolPacketSize=$cpps
	--CoverNPackets=$cnp
	--CoverInterval=$ci
	--CoverPacketSize=$cps
	--Mode=$mode
	--Debug=0
	--IntervalRatio=$incRate
    --ProbeName=$filename.csv
	--ManifestName=$filename.config
	" 2>$filename.dat
}




function IncrementalIntervals() {
	intervals=(0.1 0.01 0.001 0.0001 0.00001 0.000001 0.0000001)
	ClientProtocolNPackets=4096
	ClientProtocolInterval=0.01
	ClientProtocolPacketSize=1024
	CoverNPackets=1000000
	CoverInterval=$intervals
	CoverPacketSize=1024
	let 'exp=0'
	for i in `seq  0.0000001 -0.00000001 0.00000001`; do
		runExperiment $ClientProtocolNPackets $ClientProtocolInterval $ClientProtocolPacketSize $CoverNPackets $i $CoverPacketSize $exp &
		let 'exp=exp+1'
	done
}

function RunAndMove() {
	echo "running and moving"
	runExperiment $1 $2 $3 $4 $5 $6 $7 $8 ${10}
	finalLoc=$9
	filename=exp_$7_$1-$2-$3-$4-$5-$6-$8-${10}.dat
	mv $filename $finalLoc
}

datetime=`date "+%F_%T"`

echo $1

if [[ $1 == "debug" ]];then
	echo "debugging"
	runExperiment 1 1.0 128 1000 1.0 4096 "debug" 1 0.999
	exit 0
elif [[ $1 == "incrementalIntervals" ]]; then
	echo "running incremental intervals trial"
	IncrementalIntervals
	exit 0
elif [[ $1 == "DvUDP" ]]; then
	echo "D redundancy vs UDP"
	#runExperiment 1 1.0 128 500 1.0 4096 "dred" 0 data/backoff/echo3.dat
	totalPackets=1000000
	RunAndMove 0 1.0 128 $totalPackets 1.0 4096 "echo" 0 "data/backoff/echo_$datetime.dat" 0.9999
	RunAndMove 0 1.0 128 $totalPackets 1.0 4096 "dred" 1 "data/backoff/dred_$datetime.dat" 0.9999
	ln -sf "echo_$datetime.dat" "data/backoff/echo_latest.dat"
	ln -sf "dred_$datetime.dat" "data/backoff/dred_latest.dat"

	cd plot/variableRate
	./plotscript.sh
	exit 0
elif [[ $1 == "DvUDP-I" ]]; then
	echo "D redundancy vs UDP"
	#runExperiment 1 1.0 128 500 1.0 4096 "dred" 0 data/backoff/echo3.dat
	totalPackets=1000
	for i in `seq  0.999 -0.002 0.99`; do
		runExperiment 1 1.0 128 $totalPackets 1.0 4096 "dred" 1 $i &
		sleep 3
		runExperiment 1 1.0 128 $totalPackets 1.0 4096 "echo" 0 $i &
		sleep 3
	done
	#mv *.dat data/variableRate/
	exit 0
	
fi


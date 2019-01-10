#!/bin/bash

echo "Hello Experiment"

./waf --run \
	"scratch/pfattree
	--ClientProtocolNPackets=5
	--ClientProtocolInterval=0.1
	--ClientProtocolPacketSize=128
	--CoverNPackets=10
	--CoverInterval=0.2
	--CoverPacketSize=256
	"
		

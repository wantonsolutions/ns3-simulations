
#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"

#include "ns3/ipv4-packet-info-tag.h"
#include "raid-client.h"
#include "raid.h"

namespace ns3 {

RaidState*
InitRaidState(int parallel) {
	RaidState *rs = new RaidState;
	
	rs->Served_Raid_Requests = new bool*[RAID_REQUEST_SIZE];
	rs->Served_Raid_Packets = new Ptr<Packet>*[RAID_REQUEST_SIZE];
	for (int i=0;i<RAID_REQUEST_SIZE;i++) {
		rs->Served_Raid_Requests[i] = new bool[parallel];
		rs->Served_Raid_Packets[i] = new Ptr<Packet>[parallel];
		for (int j=0;j<parallel;j++) {
		rs->Served_Raid_Requests[i][j] = false;
		}
	}
	return rs;
}

Ptr<Packet> RaidReceive(Ptr<Packet> packet, Address from, RaidState *rs, int parallel) {
      Ipv4PacketInfoTag idtag;
      packet->PeekPacketTag(idtag);
      //TODO maintain high and low watermark with at seperate tag
      int requestIndex = idtag.GetRecvIf();
      int hitIndex = GetHitIndex(from,requestIndex);
      rs->Served_Raid_Requests[requestIndex][hitIndex] = true;
      rs->Served_Raid_Packets[requestIndex][hitIndex] = packet;
      int state = GetRaidFlowState(requestIndex,parallel,rs);
      Ptr<Packet> p;
      switch (state) {
	case RAID_INCOMPLETE:
		return NULL;
	case RAID_FIXABLE:
		p = FixPacket(requestIndex,parallel,rs);
	case RAID_COMPLETE:
		p = MergePacket(requestIndex,parallel,rs);
		//NS_LOG_INFO("Server Reconstructed Raid Packet %d requestIndex Data: %s"requestIndex, p->ToString());
      }
      p->AddPacketTag(idtag);
      return p;

}

Ptr<Packet>* StripePacket(int parallel, uint32_t size, uint32_t sent, uint8_t* data) {

  Ptr<Packet> *packets = new Ptr<Packet> [parallel];
  //printf("Size %d Parallel %d size mod %d\n",size,parallel,size%(parallel-1));
  NS_ASSERT_MSG(size % (parallel - 1) == 0, "Raid only works on data which can be equally striped across packets and parity. Assert - > DataSize % (Parallel - 1) = 0");

  int raidPacketSize = size / (parallel - 1);
  uint8_t **dataStripes = new uint8_t *[parallel];
  for (int i=0;i<parallel;i++) {
	  dataStripes[i] = new uint8_t[raidPacketSize];
  }
  //cpy raided components
  for (int i=0;i<(parallel-1);i++) {
	  memcpy(dataStripes[i],&data[i*raidPacketSize],raidPacketSize);
  }
  //Calculate the raide byte
  for (int i=0;i<raidPacketSize;i++) {
	  uint8_t rchunk = dataStripes[0][i];
	  for (int j=1;j<(parallel-1);j++) {
		  rchunk = rchunk ^ dataStripes[j][i];
	  }
	  dataStripes[parallel-1][i] = rchunk;
  }
  //Load the packets
  for (int i=0;i<parallel;i++) {
          packets[i] = Create<Packet> (dataStripes[i], raidPacketSize);
	  Ipv4PacketInfoTag idtag;
	  idtag.SetRecvIf(sent);
	  packets[i]->AddPacketTag(idtag);
  }
  return packets;
}

//RECEVING functions
int GetHitIndex(Address from, int requestIndex) {
	InetSocketAddress addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
	int mask = 0x00FF0000;
	int hitIndex = ((addr.GetIpv4().Get() & mask) >> 16) -1 ;
	return hitIndex;
}

int GetRaidFlowState(int requestIndex, int parallel, RaidState *rs) {
	int totalReceived = 0;
	for (int i =0;i<parallel-1;i++) {
		if (rs->Served_Raid_Requests[requestIndex][i]) {
			totalReceived++;
		}
	}
	//printf("Total Received for %d is %d\n",requestIndex,totalReceived );
	//all packets have been received
	if (totalReceived == parallel-1) {
		return RAID_COMPLETE;
	//n-1 parity bits have been received, and the parity channel is received
	} else if (totalReceived == parallel-2 && rs->Served_Raid_Requests[requestIndex][parallel -1]) {
		return RAID_FIXABLE;
	}
	//Not enough of the raid packets have been received to fix the packet.
	return RAID_INCOMPLETE;
}

Ptr<Packet>
FixPacket(int requestIndex, int parallel, RaidState *rs) {
	//find missing index
	int missingIndex;
	for (int i =0;i<(parallel-1);i++) {
		if (!rs->Served_Raid_Requests[requestIndex][i]) {
			missingIndex=i;
			break;
		}
	}
	//use the parity index to determine length because we know we have it
	int raidPacketSize = rs->Served_Raid_Packets[requestIndex][parallel-1]->GetSize();
	//Alloc buffers to hold packet data for the raid correction computation
	uint8_t *repairsbuf = new uint8_t[raidPacketSize];
	uint8_t **receivedpackets = new uint8_t*[parallel-1];
	for (int i=0;i<parallel-1;i++) {
		receivedpackets[i] = new uint8_t[raidPacketSize];
	}
	//working index indexes into the array of received packets, not rs->Served_Raid_Packets
	int workingIndex = 0;
	for (int i=0;i<parallel;i++) {
		if (i == missingIndex) {
			continue;
		}
		rs->Served_Raid_Packets[requestIndex][i]->CopyData(receivedpackets[workingIndex],raidPacketSize);
		workingIndex++;
	}
	  //Calculate the raide byte
	  for (int i=0;i<raidPacketSize;i++) {
		  uint8_t rchunk = receivedpackets[0][i];
		  for (int j=1;j<(parallel-1);j++) {
			  rchunk = rchunk ^ receivedpackets[j][i];
		  }
		  repairsbuf[i] = rchunk;
	  }
	  rs->Served_Raid_Packets[requestIndex][missingIndex] = new Packet(repairsbuf,raidPacketSize);
	  return MergePacket(requestIndex,parallel,rs);
}

Ptr<Packet> MergePacket(int requestIndex, int parallel, RaidState *rs) {
	//Merge the data from the first parallel - 1 packets into a single one.
	int raidPacketSize = rs->Served_Raid_Packets[requestIndex][0]->GetSize();
	uint8_t *buf = new uint8_t[(parallel-1) * raidPacketSize];
	for (int i = 0; i < (parallel-1); i++) {
		int n = rs->Served_Raid_Packets[requestIndex][i]->CopyData(&buf[i*raidPacketSize],raidPacketSize);
		if ( n != raidPacketSize ) {
			//this is panic mode because there is some data missing somewhere
			printf("Data Missing from raided packet %d continuing",i);
		}
	}
	//printf("%s\n",buf);
	Ptr<Packet> p =  new Packet(buf,raidPacketSize*(parallel-1));
	return p;

}

void RaidWrite(int requestIndex, RaidState *rs, Ptr<Packet>* packets, Ptr<Socket> socket, Address to, int parallel) {
	//This function calculates the IP's that the packet was not received on, and
	//broadcasts the packet back over those channels.
	InetSocketAddress addr = InetSocketAddress::ConvertFrom (to).GetIpv4 ();
	//NS_LOG_INFO( "Address value " << addr.GetIpv4().Get() );

	//The third unit of the IP address is used for paralleization i.e.
	//X.Y.X.X, the Y digit will be used to broadcast in parallel. This
	//function determines which of the parallel chanels the packet was
	//received on and then broadcasts across the rest.
	
	int invmask = 0xFF00FFFF;
	//NS_LOG_INFO( "Addr Key " << hitIndex);
	//TODO there is probably a cleaner way to do this by just casting the to address rather than re-initalizing
	for (int i =1; i <= parallel-1; i++) {
		//printf("Server Sending \n");
		int newAddr32 = (addr.GetIpv4().Get() & invmask) + (i << 16);
		Ipv4Address tmpAddr = addr.GetIpv4();
		tmpAddr.Set(newAddr32);
		addr.SetIpv4(tmpAddr);
		//addr.SetPort(InetSocketAddress::ConvertFrom (to).GetPort ());
		addr.SetPort(19);
		//socket->SendTo (packets[i-1], 0, addr);
		socket->SendTo(rs->Served_Raid_Packets[requestIndex][i-1],0,addr);
		//VerboseServerSendPrint(addr,packets[i-1]);
	}
}

}// ns3 namespace

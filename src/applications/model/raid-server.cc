/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/ipv6-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "ns3/ipv4-packet-info-tag.h"

#include "raid-server.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("RaidServerApplication");

NS_OBJECT_ENSURE_REGISTERED (RaidServer);

//int min, max;

TypeId
RaidServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RaidServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<RaidServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&RaidServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Parallel", "The degree to which the underlying parallel fat tree is parallelized.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&RaidServer::m_parallel),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}


RaidServer::RaidServer ()
{
  NS_LOG_FUNCTION (this);
}

RaidServer::~RaidServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  //Min and Max refer to the minumum and maximum request values received. These are yet to be implemented but will act as a high water mark in the future.
  //TODO uncomment
  //min = 0;
  //max = 0;
  //none of the requests have been served at init time, set each to false.
}

void
RaidServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
RaidServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
  //Init raid state data
  m_served_raid_requests = new bool*[SERVICE_BUFFER_SIZE];
  m_served_raid_packets = new Ptr<Packet>*[SERVICE_BUFFER_SIZE];
  for (int i=0;i<SERVICE_BUFFER_SIZE;i++) {
	  m_served_raid_requests[i] = new bool[m_parallel];
	  m_served_raid_packets[i] = new Ptr<Packet>[m_parallel];
	  for (int j=0;j<m_parallel;j++) {
	  	m_served_raid_requests[i][j] = false;
	  }
  }
  //Initalize parallel sockets
  m_sockets = new Ptr<Socket>[m_parallel];
  for (int i=0;i<m_parallel;i++) {
	printf("Starting application setting up socket %d\n",i);
	//connect a seperate socket to each of the net devices attacehd to the server node
	Ptr<Node> n = GetNode();
	Ptr<NetDevice> dev = n->GetDevice(i);
	m_sockets[i] = ConnectSocket(m_port,dev);
	m_sockets[i]->SetRecvCallback (MakeCallback (&RaidServer::HandleRead, this));
	m_sockets[i]->SetAllowBroadcast (true);
	//TODO remove if a single socket does not cut it
	break;
	//This may still be usefull, buf for the moment only a single socket is used while listening because all of the client traffic is forwarded to it.


  }

}

Ptr<Socket>
RaidServer::ConnectSocket(uint16_t port, Ptr<NetDevice> dev) {
  Ptr<Socket> socket;
  if (socket == 0)
    {
      NS_LOG_INFO("Setting up server sockets\n");
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      socket = Socket::CreateSocket (GetNode (), tid);

      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), port);
      if (socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
    }
    return socket;
}

void 
RaidServer::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
  if (m_socket6 != 0) 
    {
      m_socket6->Close ();
      m_socket6->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
    }
}

void
RaidServer::BroadcastWrite(Ptr<Packet> packet, Ptr<Socket> socket, Address from) {
	//This function calculates the IP's that the packet was not received on, and
	//broadcasts the packet back over those channels.
	InetSocketAddress addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
	NS_LOG_INFO( "Address value " << addr.GetIpv4().Get() );

	//The third unit of the IP address is used for paralleization i.e.
	//X.Y.X.X, the Y digit will be used to broadcast in parallel. This
	//function determines which of the parallel chanels the packet was
	//received on and then broadcasts across the rest.
	
	int mask = 0x00FF0000;
	int invmask = 0xFF00FFFF;
	int hitIndex = ((addr.GetIpv4().Get() & mask) >> 16);
	NS_LOG_INFO( "Addr Key " << hitIndex);
	//TODO there is probably a cleaner way to do this by just casting the from address rather than re-initalizing
	for (int i =1; i <= m_parallel; i++) {
		int newAddr32 = (addr.GetIpv4().Get() & invmask) + (i << 16);
		Ipv4Address tmpAddr = addr.GetIpv4();
		tmpAddr.Set(newAddr32);
		addr.SetIpv4(tmpAddr);
		addr.SetPort(InetSocketAddress::ConvertFrom (from).GetPort ());
		socket->SendTo (packet, 0, addr);
		VerboseServerSendPrint(addr,packet);
	}
}


int
RaidServer::GetHitIndex(Address from, int requestIndex) {
	InetSocketAddress addr = InetSocketAddress::ConvertFrom (from).GetIpv4 ();
	NS_LOG_INFO( "Address value " << addr.GetIpv4().Get() );
	int mask = 0x00FF0000;
	int hitIndex = ((addr.GetIpv4().Get() & mask) >> 16) -1 ;
	return hitIndex;
}

int
RaidServer::GetRaidFlowState(int requestIndex) {
	int totalReceived = 0;
	for (int i =0;i<m_parallel-1;i++) {
		if (m_served_raid_requests[requestIndex][i]) {
			totalReceived++;
		}
	}
	NS_LOG_INFO("Total Received for " << requestIndex << " is " << totalReceived );
	//all packets have been received
	if (totalReceived == m_parallel-1) {
		return RAID_COMPLETE;
	//n-1 parity bits have been received, and the parity channel is received
	} else if (totalReceived == m_parallel-2 && m_served_raid_requests[requestIndex][m_parallel -1]) {
		NS_LOG_INFO("RAID_FIXABLE");
		return RAID_FIXABLE;
	}
	//Not enough of the raid packets have been received to fix the packet.
	return RAID_INCOMPLETE;
}

Ptr<Packet>
RaidServer::FixPacket(int requestIndex) {
	NS_LOG_INFO("Fixing Packet Request " << requestIndex );
	//find missing index
	int missingIndex;
	for (int i =0;i<(m_parallel-1);i++) {
		if (!m_served_raid_requests[requestIndex][i]) {
			missingIndex=i;
			break;
		}
	}
	//use the parity index to determine length because we know we have it
	int raidPacketSize = m_served_raid_packets[requestIndex][m_parallel-1]->GetSize();
	//Alloc buffers to hold packet data for the raid correction computation
	uint8_t *repairsbuf = new uint8_t[raidPacketSize];
	uint8_t **receivedpackets = new uint8_t*[m_parallel-1];
	for (int i=0;i<m_parallel-1;i++) {
		receivedpackets[i] = new uint8_t[raidPacketSize];
	}
	//working index indexes into the array of received packets, not m_served_raid_packets
	int workingIndex = 0;
	for (int i=0;i<m_parallel;i++) {
		if (i == missingIndex) {
			continue;
		}
		m_served_raid_packets[requestIndex][i]->CopyData(receivedpackets[workingIndex],raidPacketSize);
		workingIndex++;
	}
	  //Calculate the raide byte
	  for (int i=0;i<raidPacketSize;i++) {
		  uint8_t rchunk = receivedpackets[0][i];
		  for (int j=1;j<(m_parallel-1);j++) {
			  rchunk = rchunk ^ receivedpackets[j][i];
		  }
		  repairsbuf[i] = rchunk;
	  }
	  m_served_raid_packets[requestIndex][missingIndex] = new Packet(repairsbuf,raidPacketSize);
	  return MergePacket(requestIndex);
}

Ptr<Packet>
RaidServer::MergePacket(int requestIndex) {
	//Merge the data from the first m_parallel - 1 packets into a single one.
	int raidPacketSize = m_served_raid_packets[requestIndex][0]->GetSize();
	uint8_t *buf = new uint8_t[(m_parallel-1) * raidPacketSize];
	for (int i = 0; i < (m_parallel-1); i++) {
		int n = m_served_raid_packets[requestIndex][i]->CopyData(&buf[i*raidPacketSize],raidPacketSize);
		if ( n != raidPacketSize ) {
			//this is panic mode because there is some data missing somewhere
			NS_LOG_INFO("Data Missing from raided packet "<< i << "continuing");
		}
	}
	printf("%s\n",buf);
	Ptr<Packet> p =  new Packet(buf,raidPacketSize*m_parallel);
	return p;

}


void 
RaidServer::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);

  Ptr<Packet> packet;
  Address from;

  while ((packet = socket->RecvFrom (from)))
    {

       VerboseServerReceivePrint(from,packet);

      //packet->RemoveAllPacketTags ();
      //packet->RemoveAllByteTags ();
      
      Ipv4PacketInfoTag idtag;
      packet->PeekPacketTag(idtag);
      //TODO maintain high and low watermark with at seperate tag
      int requestIndex = idtag.GetRecvIf();
      int hitIndex = GetHitIndex(from,requestIndex);
      m_served_raid_requests[requestIndex][hitIndex] = true;
      m_served_raid_packets[requestIndex][hitIndex] = packet;
      int state = GetRaidFlowState(requestIndex);
      Ptr<Packet> p;
      switch (state) {
	case RAID_INCOMPLETE:
		NS_LOG_INFO("Request " << requestIndex << "Still waiting for data");
		continue;
	case RAID_FIXABLE:
		p = FixPacket(requestIndex);
	        BroadcastWrite(p,socket,from);
	        VerboseServerSendPrint(from,packet);
	case RAID_COMPLETE:
		continue;
		NS_LOG_INFO("RAID_COMPLETE returned preparing to merge packet");
		p = MergePacket(requestIndex);
		NS_LOG_INFO("Server Reconstructed Raid Packet " << requestIndex << " Data:" << p->ToString());
	        BroadcastWrite(p,socket,from);
	      	VerboseServerSendPrint(from,packet);
      }

   }
}

void RaidServer::VerboseServerReceivePrint(Address from, Ptr<Packet> packet) {
      if (InetSocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
		       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
		       InetSocketAddress::ConvertFrom (from).GetPort ());
	}
      else if (Inet6SocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server received " << packet->GetSize () << " bytes from " <<
		       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
		       Inet6SocketAddress::ConvertFrom (from).GetPort ());
	}

}

void RaidServer::VerboseServerSendPrint(Address from, Ptr<Packet> packet) {
      if (InetSocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
		       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
		       InetSocketAddress::ConvertFrom (from).GetPort ());
	}
      else if (Inet6SocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s server sent " << packet->GetSize () << " bytes to " <<
		       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
		       Inet6SocketAddress::ConvertFrom (from).GetPort ());
	}
}

} // Namespace ns3

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
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "raid-client.h"
#include "raid.h"

//#include "ns3/lte-pdcp-tag.h"
#include "ns3/ipv4-packet-info-tag.h"
//#include "ns3/lte-pdcp-tag.h"
#include <iostream>
#include <fstream>



namespace ns3 {
	
	Time raid_requests[RAID_REQUEST_SIZE];


NS_LOG_COMPONENT_DEFINE ("RaidClientApplication");

NS_OBJECT_ENSURE_REGISTERED (RaidClient);


TypeId
RaidClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::RaidClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<RaidClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&RaidClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&RaidClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&RaidClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&RaidClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&RaidClient::SetDataSize,
                                         &RaidClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute("Parallel", "The degree to which the undterlying parallel fat tree is parallelized",
		    UintegerValue(1),
		    MakeUintegerAccessor (&RaidClient::m_parallel),
		    MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&RaidClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

Ptr<Socket> 
RaidClient::ConnectSocket(Address address, uint16_t port, Ptr<NetDevice> dev) {
  NS_LOG_FUNCTION ("Connecting to a socket");

  Ptr<Socket> socket;
  if (socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(address) == true)
        {
          socket->BindToNetDevice (dev);
              //NS_FATAL_ERROR ("Failed to bind socket");
          socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(address), port));
        }
      else if (Ipv6Address::IsMatchingType(address) == true)
        {
          if (socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(address), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (address) == true)
        {
          if (socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          socket->Connect (address);
        }
      else if (Inet6SocketAddress::IsMatchingType (address) == true)
        {
          if (socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          socket->Connect (address);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << address);
        }
    }
    return socket;

}

  

RaidClient::RaidClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_socket = 0;
  m_parallel = 1;
  m_sendEvent = EventId();
  m_data = 0;
  m_dataSize = 0;
}

RaidClient::~RaidClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
RaidClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
RaidClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
RaidClient::SetAddresses(Address* addresses, uint8_t len)
{
	for(int i=0; i< len; i++) {
		NS_LOG_FUNCTION(this << addresses[i]);
	}
	m_peerAddresses = addresses;
	//This is a bit hacky, but I need to have an event for each addresss
}

void
RaidClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
RaidClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  m_sockets = new Ptr<Socket>[m_parallel];
  ScheduleTransmit (Seconds (0.));
  for (int i=0;i<m_parallel;i++) {
	NS_LOG_INFO("Starting application setting up socket " << i);
	Ptr<Node> n = GetNode();
	Ptr<NetDevice> dev = n->GetDevice(i);
	m_sockets[i] = ConnectSocket(m_peerAddresses[i],m_peerPort,dev);
	m_sockets[i]->SetRecvCallback (MakeCallback (&RaidClient::HandleRead, this));
	m_sockets[i]->SetAllowBroadcast (true);
  }
}


void 
RaidClient::StopApplication ()
{
  NS_LOG_FUNCTION (this);

  if (m_socket != 0) 
    {
      m_socket->Close ();
      m_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_socket = 0;
    }
  	Simulator::Cancel (m_sendEvent);
}

void 
RaidClient::SetDataSize (uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << dataSize);




  //
  // If the client is setting the echo packet data size this way, we infer
  // that she doesn't care about the contents of the packet at all, so 
  // neither will we.
  //
  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
  m_size = dataSize;
}

uint32_t 
RaidClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
RaidClient::SetFill (std::string fill)
{
  NS_LOG_FUNCTION (this << fill);

  uint32_t dataSize = fill.size () + 1;

  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memcpy (m_data, fill.c_str (), dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
RaidClient::SetFill (uint8_t fill, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  memset (m_data, fill, dataSize);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
RaidClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
{
  NS_LOG_FUNCTION (this << fill << fillSize << dataSize);
  if (dataSize != m_dataSize)
    {
      delete [] m_data;
      m_data = new uint8_t [dataSize];
      m_dataSize = dataSize;
    }

  if (fillSize >= dataSize)
    {
      memcpy (m_data, fill, dataSize);
      m_size = dataSize;
      return;
    }

  //
  // Do all but the final fill.
  //
  uint32_t filled = 0;
  while (filled + fillSize < dataSize)
    {
      memcpy (&m_data[filled], fill, fillSize);
      filled += fillSize;
    }

  //
  // Last fill may be partial
  //
  memcpy (&m_data[filled], fill, dataSize - filled);

  //
  // Overwrite packet size attribute.
  //
  m_size = dataSize;
}

void 
RaidClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &RaidClient::Send, this);
}

//void RaidClient::RegularSend (void)
//{
//
//}

void 
RaidClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  //check that the first event is expired
  //NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      NS_ASSERT_MSG (m_dataSize == m_size, "RaidClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "RaidClient::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      p = Create<Packet> (m_size);
    }
  
  Ptr<Packet> *packets = StripePacket(m_parallel, m_size, m_sent, m_data);
  raid_requests[m_sent % RAID_REQUEST_SIZE] = Simulator::Now();

  m_sent++;
  m_txTrace (p);
  for (int i=0;i<m_parallel;i++) {
  	m_sockets[i]->Send (packets[i]);
	VerboseSendLogging(m_peerAddresses[i]);
  }
  if (m_sent < m_count) 
    {
      printf("Scheduling next Transmission for %d in the future\n",int(m_interval.GetNanoSeconds()));
      ScheduleTransmit (m_interval);
    }
	
}

void
RaidClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Ipv4PacketInfoTag idtag;
  Address from;
  NS_LOG_INFO("Waiting for a socket to be received on!!");
  while ((packet = socket->RecvFrom (from)))
    {
	    printf("Client Receive!!");
      if (packet->PeekPacketTag(idtag)) {

	      //Packet takes are enumerated over a ring the size of
	      //RAID_REQUEST_SIZE 
	      //TODO develop a custom tag for the protocols
	      //which sends the max, and min values along with the request
	      //index so that the state on the server can be relieved.
	      
	      int requestIndex = int(idtag.GetRecvIf()) % RAID_REQUEST_SIZE;
	      NS_LOG_FUNCTION("request Index: " << requestIndex);
	      if (raid_requests[requestIndex] > Time(0)) {
		      NS_LOG_INFO("New Client Response " << requestIndex << " Received");
		      Time difference = Simulator::Now() - raid_requests[requestIndex];
		      raid_requests[requestIndex] = Time(0);

		      //Peers connected on port 11 are the ones being monitered. The
		      //differnece time being logged is the end to end latency of a
		      //request.
		      //TODO Add bandwidth to the measure of each request.
		      
		      if (m_peerPort == 11) {
			NS_LOG_INFO(difference.GetNanoSeconds());
		      }
	      } else {
		      NS_LOG_INFO("Old Client Response " << requestIndex << " Received");
	      }
	      VerboseReceiveLogging(from,packet);
      }
    }
}

void RaidClient::VerboseSendLogging(Address to) {
  if (Ipv4Address::IsMatchingType (to))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
		   Ipv4Address::ConvertFrom (to) << " port " << m_peerPort);
    }
  else if (Ipv6Address::IsMatchingType (to))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
		   Ipv6Address::ConvertFrom (to) << " port " << m_peerPort);
    }
  else if (InetSocketAddress::IsMatchingType (to))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
		   InetSocketAddress::ConvertFrom (to).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (to).GetPort ());
    }
  else if (Inet6SocketAddress::IsMatchingType (to))
    {
      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
		   Inet6SocketAddress::ConvertFrom (to).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (to).GetPort ());
    }
}

void
RaidClient::VerboseReceiveLogging(Address from, Ptr<Packet> packet) {
      if (InetSocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
		       InetSocketAddress::ConvertFrom (from).GetIpv4 () << " port " <<
		       InetSocketAddress::ConvertFrom (from).GetPort ());
	}
      else if (Inet6SocketAddress::IsMatchingType (from))
	{
	  NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client received " << packet->GetSize () << " bytes from " <<
		       Inet6SocketAddress::ConvertFrom (from).GetIpv6 () << " port " <<
		       Inet6SocketAddress::ConvertFrom (from).GetPort ());
	}
}
} // Namespace ns3

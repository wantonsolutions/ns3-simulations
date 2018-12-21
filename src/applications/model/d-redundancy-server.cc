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

#include "d-redundancy-server.h"

#define SERVICE_BUFFER_SIZE 4096
namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("DRedundancyServerApplication");

NS_OBJECT_ENSURE_REGISTERED (DRedundancyServer);

bool served_requests[SERVICE_BUFFER_SIZE];
int min, max;

TypeId
DRedundancyServer::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DRedundancyServer")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<DRedundancyServer> ()
    .AddAttribute ("Port", "Port on which we listen for incoming packets.",
                   UintegerValue (9),
                   MakeUintegerAccessor (&DRedundancyServer::m_port),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Parallel", "The degree to which the underlying parallel fat tree is parallelized.",
                   UintegerValue (1),
                   MakeUintegerAccessor (&DRedundancyServer::m_parallel),
                   MakeUintegerChecker<uint8_t> ())
  ;
  return tid;
}

DRedundancyServer::DRedundancyServer ()
{
  NS_LOG_FUNCTION (this);
}

DRedundancyServer::~DRedundancyServer()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;
  m_socket6 = 0;

  min = 0;
  max = 0;
  for (int i=0;i<SERVICE_BUFFER_SIZE;i++) {
	  served_requests[i] = false;
  }
}

void
DRedundancyServer::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
DRedundancyServer::StartApplication (void)
{
  NS_LOG_FUNCTION (this);
/*
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      if (m_socket6->Bind (local6) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      if (m_socket->Bind (local) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }

  if (m_socket6 == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket6 = Socket::CreateSocket (GetNode (), tid);
      Inet6SocketAddress local6 = Inet6SocketAddress (Ipv6Address::GetAny (), m_port);
      if (m_socket6->Bind (local6) == -1)
        {
          NS_FATAL_ERROR ("Failed to bind socket");
        }
      if (addressUtils::IsMulticast (local6))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (m_socket6);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, local6);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }
    }*/
  m_sockets = new Ptr<Socket>[m_parallel];
  //ScheduleTransmit (Seconds (0.));
  for (int i=0;i<m_parallel;i++) {
	printf("Starting application setting up socket %d\n",i);
	Ptr<Node> n = GetNode();
	Ptr<NetDevice> dev = n->GetDevice(i);
	m_sockets[i] = ConnectSocket(m_port,dev);
	m_sockets[i]->SetRecvCallback (MakeCallback (&DRedundancyServer::HandleRead, this));
	m_sockets[i]->SetAllowBroadcast (true);
  }

  //m_socket->SetRecvCallback (MakeCallback (&DRedundancyServer::HandleRead, this));
  //m_socket6->SetRecvCallback (MakeCallback (&DRedundancyServer::HandleRead, this));
}

Ptr<Socket>
DRedundancyServer::ConnectSocket(uint16_t port, Ptr<NetDevice> dev) {
  Ptr<Socket> socket;
  if (socket == 0)
    {
	    printf("Setting up server sockets");
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      socket = Socket::CreateSocket (GetNode (), tid);

      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      InetSocketAddress local = InetSocketAddress (dev->GetAddress(), m_port);
      socket->Bind(local) ;
	if (addressUtils::IsMulticast (local)) {
		{
		  Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (socket);
		  if (udpSocket)
		    {
		      printf("Setting up that multicast");
		      // equivalent to setsockopt (MCAST_JOIN_GROUP)
		      udpSocket->MulticastJoinGroup (0, local);
		    }
		  else
		    {
		      NS_FATAL_ERROR ("Error: Failed to join multicast group");
		    }
		}
	}

      /*	      
      if (addressUtils::IsMulticast (m_local))
        {
          Ptr<UdpSocket> udpSocket = DynamicCast<UdpSocket> (socket);
          if (udpSocket)
            {
              // equivalent to setsockopt (MCAST_JOIN_GROUP)
              udpSocket->MulticastJoinGroup (0, m_local);
            }
          else
            {
              NS_FATAL_ERROR ("Error: Failed to join multicast group");
            }
        }*/
    }
    return socket;
	
}

void 
DRedundancyServer::StopApplication ()
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
DRedundancyServer::HandleRead (Ptr<Socket> socket)
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
      int requestIndex = idtag.GetRecvIf();

      if (!served_requests[requestIndex]) {
	      NS_LOG_INFO("First time request " << requestIndex << " Has arrived, responding");
	      served_requests[requestIndex] = true;
	      //TODO broadcast back to many IP's
	      socket->SendTo (packet, 0, from);
	      VerboseServerSendPrint(from,packet);
      } else {
	      NS_LOG_INFO("Request " << requestIndex << " Has allready been serviced");
      }

   }
}

void DRedundancyServer::VerboseServerReceivePrint(Address from, Ptr<Packet> packet) {
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

void DRedundancyServer::VerboseServerSendPrint(Address from, Ptr<Packet> packet) {
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

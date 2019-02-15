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
#include "udp-echo-client.h"

//#include "ns3/lte-pdcp-tag.h"
#include "ns3/ipv4-packet-info-tag.h"
#include <iostream>
#include <fstream>


namespace ns3 {
	


NS_LOG_COMPONENT_DEFINE ("UdpEchoClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpEchoClient);

TypeId
UdpEchoClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::UdpEchoClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<UdpEchoClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpEchoClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&UdpEchoClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&UdpEchoClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&UdpEchoClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&UdpEchoClient::SetDataSize,
                                         &UdpEchoClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&UdpEchoClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

  

UdpEchoClient::UdpEchoClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_rec = 0;
  m_socket = 0;
  m_intervalRatio = 0.01;
  m_sendEvent = EventId ();
  m_data = 0;
  m_dataSize = 0;
}

UdpEchoClient::~UdpEchoClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
UdpEchoClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
UdpEchoClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
UdpEchoClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
UdpEchoClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  //TODO Start here, the first thing you need to do is split on the port
  //number. Then you are going to need to go into the addressing code, and set
  //basenumbers for each of the parallel channels. Part 1 is hard Part 2 NSM.
  //Then the tricky part, on the fly design cover traffic patterns to disrupt
  //the current traffic. The most basic would be uniform traffic, Then Zipf,
  //finally bursty. There is no way you make all of that and take measurements to prepare for failure.
  
  if (m_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_socket = Socket::CreateSocket (GetNode (), tid);
      if (Ipv4Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (InetSocketAddress (Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (Ipv6Address::IsMatchingType(m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (Inet6SocketAddress (Ipv6Address::ConvertFrom(m_peerAddress), m_peerPort));
        }
      else if (InetSocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else if (Inet6SocketAddress::IsMatchingType (m_peerAddress) == true)
        {
          if (m_socket->Bind6 () == -1)
            {
              NS_FATAL_ERROR ("Failed to bind socket");
            }
          m_socket->Connect (m_peerAddress);
        }
      else
        {
          NS_ASSERT_MSG (false, "Incompatible address type: " << m_peerAddress);
        }
    }

  m_socket->SetRecvCallback (MakeCallback (&UdpEchoClient::HandleRead, this));
  m_socket->SetAllowBroadcast (true);
  ScheduleTransmit (Seconds (0.));
}

void 
UdpEchoClient::StopApplication ()
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
UdpEchoClient::SetDataSize (uint32_t dataSize)
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
UdpEchoClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
UdpEchoClient::SetFill (std::string fill)
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
UdpEchoClient::SetFill (uint8_t fill, uint32_t dataSize)
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
UdpEchoClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
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
UdpEchoClient::ScheduleTransmit (Time dt)
{
  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &UdpEchoClient::Send, this);
}

//void UdpEchoClient::RegularSend (void)
//{
//
//}

void 
UdpEchoClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (m_dataSize == m_size, "UdpEchoClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "UdpEchoClient::Send(): m_dataSize but no m_data");
      p = Create<Packet> (m_data, m_dataSize);
    }
  else
    {
      //
      // If m_dataSize is zero, the client has indicated that it doesn't care
      // about the data itself either by specifying the data size by setting
      // the corresponding attribute or by not calling a SetFill function.  In
      // this case, we don't worry about it either.  But we do allow m_size
      // to have a value different from the (zero) m_dataSize.
      //
      p = Create<Packet> (m_size);
    }
  // call to the trace sinks before the packet is actually sent,
  // so that tags added to the packet can be sent as well
  //
  
  //PdcpTag idtag;
  Ipv4PacketInfoTag idtag;
  //idtag.SetSenderTimestamp(Time(m_sent));
  //printf("Sending Packet %d\n",m_sent);
  uint32_t send_index = m_sent % REQUEST_BUFFER_SIZE;
  idtag.SetRecvIf(send_index);
  p->AddPacketTag(idtag);
  m_requests[m_sent % REQUEST_BUFFER_SIZE] = Simulator::Now();

    // inserting values by using [] operator 
    //umap["GeeksforGeeks"] = 10; 
    //umap["Practice"] = 20; 
    //umap["Contribute"] = 30; 

  m_txTrace (p);
  m_socket->Send (p);

  ++m_sent;
	  if (Ipv4Address::IsMatchingType (m_peerAddress))
	    {
	      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
			   Ipv4Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
	    }
	  else if (Ipv6Address::IsMatchingType (m_peerAddress))
	    {
	      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
			   Ipv6Address::ConvertFrom (m_peerAddress) << " port " << m_peerPort);
	    }
	  else if (InetSocketAddress::IsMatchingType (m_peerAddress))
	    {
	      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
			   InetSocketAddress::ConvertFrom (m_peerAddress).GetIpv4 () << " port " << InetSocketAddress::ConvertFrom (m_peerAddress).GetPort ());
	    }
	  else if (Inet6SocketAddress::IsMatchingType (m_peerAddress))
	    {
	      NS_LOG_INFO ("At time " << Simulator::Now ().GetSeconds () << "s client sent " << m_size << " bytes to " <<
			   Inet6SocketAddress::ConvertFrom (m_peerAddress).GetIpv6 () << " port " << Inet6SocketAddress::ConvertFrom (m_peerAddress).GetPort ());
	    }
	  //printf("Sent %d , count %d\n",m_sent, m_count);
  if (m_sent < m_count) 
    {
      m_interval = SetInterval();
      ScheduleTransmit (m_interval);
    }
	
}

void UdpEchoClient::SetIntervalRatio(double ratio) {
	m_intervalRatio = ratio;
}

Time UdpEchoClient::SetInterval() {
  Time interval;
  switch (m_dist) {
  case nodist:
	  {
		interval = m_interval;
		break;
	  }
	  case incremental:
	  {
		double nextTime = incrementalDistributionNext((double) m_interval.GetSeconds(), 0.99);
		//printf("Current Interval - %f, next Interval %f\n",(float)m_interval.GetSeconds(), nextTime);
		interval = Time(Seconds(nextTime));
		break;
	  }
	  case evenuniform:
	  {
		double nextTime = evenUniformDistributionNext(0, 0);
		interval = Time(Seconds(nextTime));
		break;
	  }
	  case exponential:
	  {
		double nextTime = (double) exponentailDistributionNext(0, 0);
		interval = Time(Seconds(nextTime));
		break;
	  }
	  case possion:
	  {
		double nextTime = (double) poissonDistributionNext(0, 0);
		interval = Time(Seconds(nextTime));
		break;
	  }
  }
  return interval;

}

void
UdpEchoClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  //PdcpTag idtag;
  Ipv4PacketInfoTag idtag;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->PeekPacketTag(idtag)) {

	       m_rec++;
	      //NS_LOG_INFO("Tag ID" << idtag.GetSenderTimestamp());
	      //NS_LOG_INFO("timestamp index " << idtag.GetSenderTimestamp().GetNanoSeconds());
	      //int requestIndex = int(idtag.GetSenderTimestamp().GetNanoSeconds()) % REQUEST_BUFFER_SIZE;
	      int requestIndex = int(idtag.GetRecvIf()) % REQUEST_BUFFER_SIZE;
              Time difference = Simulator::Now() - m_requests[requestIndex];
	      	//NS_LOG_WARN(difference.GetNanoSeconds());
		//
		//       NS_LOG_WARN(difference.GetNanoSeconds() << "," <<
		//		       Simulator::Now ().GetSeconds ()); 
		  
	      NS_LOG_WARN(difference.GetNanoSeconds() << "," <<
				       Simulator::Now ().GetSeconds () << "," <<
				       m_sent << "," <<
				       m_rec << "," <<
				       requestIndex << "," <<
				       0 << ","
				       ); 
	      
      }
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
}

  void UdpEchoClient::SetDistribution(enum distribution dist){
	  m_dist = dist;
  }

  double UdpEchoClient::incrementalDistributionNext(double current, double rate) {
	  double nextRate = current * rate;
	  double percentbound = nextRate * 0.1;
	  double fMin = 0.0 - percentbound;
	  double fMax = 0.0 + percentbound;
	  double f = (double)rand() / RAND_MAX;
	  double offset = fMin + f * (fMax - fMin);
	  double ret = nextRate + offset;
	  //printf("New Rate %f\n",ret);
	  return ret;
	
  }

  int UdpEchoClient::evenUniformDistributionNext(int min, int max) {
	  return 0;
  }

  int UdpEchoClient::exponentailDistributionNext(int min, int max) {
	  return 0;
  }

  int UdpEchoClient::poissonDistributionNext(int min, int max) {
	  return 0;
  }

} // Namespace ns3

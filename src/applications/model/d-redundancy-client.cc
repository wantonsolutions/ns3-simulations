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
#include "d-redundancy-client.h"

//#include "ns3/lte-pdcp-tag.h"
#include "ns3/ipv4-packet-info-tag.h"
//#include "ns3/lte-pdcp-tag.h"
#include <iostream>
#include <fstream>



namespace ns3 {
	


NS_LOG_COMPONENT_DEFINE ("DRedundancyClientApplication");

NS_OBJECT_ENSURE_REGISTERED (DRedundancyClient);


TypeId
DRedundancyClient::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::DRedundancyClient")
    .SetParent<Application> ()
    .SetGroupName("Applications")
    .AddConstructor<DRedundancyClient> ()
    .AddAttribute ("MaxPackets", 
                   "The maximum number of packets the application will send",
                   UintegerValue (100),
                   MakeUintegerAccessor (&DRedundancyClient::m_count),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute ("Interval", 
                   "The time to wait between packets",
                   TimeValue (Seconds (1.0)),
                   MakeTimeAccessor (&DRedundancyClient::m_interval),
                   MakeTimeChecker ())
    .AddAttribute ("RemoteAddress", 
                   "The destination Address of the outbound packets",
                   AddressValue (),
                   MakeAddressAccessor (&DRedundancyClient::m_peerAddress),
                   MakeAddressChecker ())
    .AddAttribute ("RemotePort", 
                   "The destination port of the outbound packets",
                   UintegerValue (0),
                   MakeUintegerAccessor (&DRedundancyClient::m_peerPort),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("PacketSize", "Size of echo data in outbound packets",
                   UintegerValue (100),
                   MakeUintegerAccessor (&DRedundancyClient::SetDataSize,
                                         &DRedundancyClient::GetDataSize),
                   MakeUintegerChecker<uint32_t> ())
    .AddAttribute("Parallel", "The degree to which the undterlying parallel fat tree is parallelized",
		    UintegerValue(1),
		    MakeUintegerAccessor (&DRedundancyClient::m_parallel),
		    MakeUintegerChecker<uint16_t> ())
    .AddTraceSource ("Tx", "A new packet is created and is sent",
                     MakeTraceSourceAccessor (&DRedundancyClient::m_txTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}

Ptr<Socket> 
DRedundancyClient::ConnectSocket(Address address, uint16_t port, Ptr<NetDevice> dev) {
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

  

DRedundancyClient::DRedundancyClient ()
{
  NS_LOG_FUNCTION (this);
  m_sent = 0;
  m_rec = 0;
  m_socket = 0;
  m_parallel = 1;
  m_intervalRatio = 0.01;
  m_sendEvent = EventId();
  m_data = 0;
  m_dataSize = 0;
}

DRedundancyClient::~DRedundancyClient()
{
  NS_LOG_FUNCTION (this);
  m_socket = 0;

  delete [] m_data;
  m_data = 0;
  m_dataSize = 0;
}

void 
DRedundancyClient::SetRemote (Address ip, uint16_t port)
{
  NS_LOG_FUNCTION (this << ip << port);
  m_peerAddress = ip;
  m_peerPort = port;
}

void 
DRedundancyClient::SetRemote (Address addr)
{
  NS_LOG_FUNCTION (this << addr);
  m_peerAddress = addr;
}

void
DRedundancyClient::SetAddresses(Address* addresses, uint8_t len)
{
	for(int i=0; i< len; i++) {
		NS_LOG_FUNCTION(this << addresses[i]);
	}
	m_peerAddresses = addresses;
	m_parallel = len;
	//This is a bit hacky, but I need to have an event for each addresss
}

void
DRedundancyClient::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  Application::DoDispose ();
}

void 
DRedundancyClient::StartApplication (void)
{
  NS_LOG_FUNCTION (this);

  m_sockets = new Ptr<Socket>[m_parallel];
  ScheduleTransmit (Seconds (0.));
  m_parallel = 3;
  m_d_level = m_parallel;
  m_minRTT = 999999999;
  m_root_channel = rand()%m_parallel;
  NS_LOG_INFO("D-Red client PARALLEL = " << m_parallel);
  for (int i=0;i<m_parallel;i++) {
	NS_LOG_INFO("Starting application setting up socket " << i);
	//printf("Random Starting Channel %d\n",rand()%m_parallel);
	Ptr<Node> n = GetNode();
	Ptr<NetDevice> dev = n->GetDevice(i);
	m_sockets[i] = ConnectSocket(m_peerAddresses[i],m_peerPort,dev);
	m_sockets[i]->SetRecvCallback (MakeCallback (&DRedundancyClient::HandleRead, this));
	m_sockets[i]->SetAllowBroadcast (true);
  }
}


void 
DRedundancyClient::StopApplication ()
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
DRedundancyClient::SetDataSize (uint32_t dataSize)
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
DRedundancyClient::GetDataSize (void) const
{
  NS_LOG_FUNCTION (this);
  return m_size;
}

void 
DRedundancyClient::SetFill (std::string fill)
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
DRedundancyClient::SetFill (uint8_t fill, uint32_t dataSize)
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
DRedundancyClient::SetFill (uint8_t *fill, uint32_t fillSize, uint32_t dataSize)
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
DRedundancyClient::ScheduleTransmit (Time dt)
{

  NS_LOG_FUNCTION (this << dt);
  m_sendEvent = Simulator::Schedule (dt, &DRedundancyClient::Send, this);
}

//void DRedundancyClient::RegularSend (void)
//{
//
//}

void 
DRedundancyClient::Send (void)
{
  NS_LOG_FUNCTION (this);

  //check that the first event is expired
  //NS_ASSERT (m_sendEvent.IsExpired ());

  Ptr<Packet> p;
  if (m_dataSize)
    {
      //
      // If m_dataSize is non-zero, we have a data buffer of the same size that we
      // are expected to copy and send.  This state of affairs is created if one of
      // the Fill functions is called.  In this case, m_size must have been set
      // to agree with m_dataSize
      //
      NS_ASSERT_MSG (m_dataSize == m_size, "DRedundancyClient::Send(): m_size and m_dataSize inconsistent");
      NS_ASSERT_MSG (m_data, "DRedundancyClient::Send(): m_dataSize but no m_data");
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
  uint32_t send_index = m_sent % REQUEST_BUFFER_SIZE;
  idtag.SetRecvIf(send_index);
  NS_LOG_INFO("request Index: " <<idtag.GetRecvIf());
  //printf("Sending Packet %d\n",m_sent);
  p->AddPacketTag(idtag);
  m_d_requests[send_index] = Simulator::Now();
  m_d_requests_received[send_index] = false;

    // inserting values by using [] operator 
    //umap["GeeksforGeeks"] = 10; 
    //umap["Practice"] = 20; 
    //umap["Contribute"] = 30; 

  m_sent++;
  m_txTrace (p);
  for (int i=0;i<m_parallel;i++) {
	if (( (i + m_root_channel) % m_parallel) < m_d_level) {
		m_sockets[i]->Send (p);
		VerboseSendLogging(m_peerAddresses[i]);
  	}
  }
  if (m_sent < m_count) 
    {
      //printf("Scheduling next Transmission for %d in the future\n",int(m_interval.GetNanoSeconds()));
      m_interval = SetInterval();
      ScheduleTransmit (m_interval);
    }
	
}

void DRedundancyClient::SetIntervalRatio(double ratio) {
	m_intervalRatio = ratio;
}

Time DRedundancyClient::SetInterval() {
  Time interval;
  switch (m_dist) {
  case nodist:
	  {
		interval = m_interval;
		break;
	  }
	  case incremental:
	  {
		double nextTime = incrementalDistributionNext((double) m_interval.GetSeconds(), m_intervalRatio);
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
DRedundancyClient::HandleRead (Ptr<Socket> socket)
{
  NS_LOG_FUNCTION (this << socket);
  Ptr<Packet> packet;
  Ipv4PacketInfoTag idtag;
  Address from;
  while ((packet = socket->RecvFrom (from)))
    {
      if (packet->PeekPacketTag(idtag)) {

	      //Packet takes are enumerated over a ring the size of
	      //REQUEST_BUFFER_SIZE 
	      //TODO develop a custom tag for the protocols
	      //which sends the max, and min values along with the request
	      //index so that the state on the server can be relieved.
	      
	      int requestIndex = int(idtag.GetRecvIf()) % REQUEST_BUFFER_SIZE;
	      int rttIndex = int(idtag.GetRecvIf()) % RTT_BUFFER_SIZE;
	      
	      NS_LOG_FUNCTION("request Index: " << requestIndex);
	      if (m_d_requests_received[requestIndex] == false) {
		      m_rec++;
		      m_d_requests_received[requestIndex] = true;
		      NS_LOG_INFO("New Client Response " << requestIndex << " Received");
		      Time now = Simulator::Now();
		      Time difference = now - m_d_requests[requestIndex];
		      NS_LOG_INFO("Index " << requestIndex << "Start " << m_d_requests[requestIndex].GetNanoSeconds() << " Finish " << now.GetNanoSeconds() << " Difference " << difference.GetNanoSeconds());

		      m_recentRTTs[rttIndex] = difference;

		      //track minimum
		      if (difference.GetNanoSeconds() < m_minRTT) {
			      m_minRTT = difference.GetNanoSeconds();
		      }
		      //Calculate a bound on the average computation
		      int bound = RTT_BUFFER_SIZE;
		      if (bound > requestIndex) {
			      bound = requestIndex+1;
		      }
		      //printf("bound calculatedi %d\n", bound);
		      
		      //Calculate average rtt
		      int64_t average = 0;
		      for (int i =0; i< bound;i++) {
			      average += m_recentRTTs[i].GetNanoSeconds();
		      }
		      average = average / bound;
		      //printf("average calculated %ld\n", average);

		      //cacluate standard dev
		      int64_t diffSquare = 0;
		      for (int i =0; i< bound;i++) {
			      diffSquare += (average - m_recentRTTs[i].GetNanoSeconds()) * (average - m_recentRTTs[i].GetNanoSeconds());
		      }
		      //printf("diff square calculated %ld\n",diffSquare);
		      diffSquare = diffSquare / bound;
		      //printf("bound %d\n",bound);
		      //Cheap square
		      /*
		      int64_t std = 0;
		      for (int64_t i=0;(i*i) < diffSquare;i++) {
			      std = i;
		      }
		      */
		      //printf("min %ld, average %ld, std %ld level m_d_level %d \n",m_minRTT,average,std,m_d_level);
		      //Make a decision to pull back
		      //if (average > (m_minRTT + std) && m_d_level > 1) {
		      if (average > (m_minRTT + (m_minRTT/10)) && m_d_level > 1) {
			      //printf("Pulling Back from D level %d to %d",m_d_level,m_d_level - 1);
			      m_d_level--;
		      //} else if (average < (m_minRTT + std) && m_d_level < m_parallel) {
		      } else if (average < (m_minRTT + (m_minRTT/10)) && m_d_level < m_parallel) {
			      //printf("Upgrading from from D level %d to %d",m_d_level,m_d_level + 1);
			      m_d_level++;
		      }
		      //Peers connected on port 11 are the ones being monitered. The
		      //differnece time being logged is the end to end latency of a
		      //request.
		      //TODO Add bandwidth to the measure of each request.
		       //NS_LOG_WARN(difference.GetNanoSeconds() << "-" << m_sent);
		       //NS_LOG_WARN(difference.GetNanoSeconds()); 
	      NS_LOG_WARN(difference.GetNanoSeconds() << "," <<
				       Simulator::Now ().GetSeconds () << "," <<
				       m_sent << "," <<
				       m_rec << "," <<
				       requestIndex << "," <<
				       (int) m_d_level << ","
				       ); 
	      } else {
		      NS_LOG_INFO("Old Client Response " << requestIndex << " Received");
	      }

	      VerboseReceiveLogging(from,packet);
      }
    }
}

void DRedundancyClient::VerboseSendLogging(Address to) {
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
DRedundancyClient::VerboseReceiveLogging(Address from, Ptr<Packet> packet) {
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


  void DRedundancyClient::SetDistribution(enum distribution dist){
	  m_dist = dist;
  }

  double DRedundancyClient::incrementalDistributionNext(double current, double rate) {
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

  int DRedundancyClient::evenUniformDistributionNext(int min, int max) {
	  return 0;
  }

  int DRedundancyClient::exponentailDistributionNext(int min, int max) {
	  return 0;
  }

  int DRedundancyClient::poissonDistributionNext(int min, int max) {
	  return 0;
  }

} // Namespace ns3

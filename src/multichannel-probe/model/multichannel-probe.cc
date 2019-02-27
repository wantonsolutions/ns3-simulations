/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016 University of California, San Diego
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
 *
 * Authors:  Alex Forencich <alex@alexforencich.com>
 */

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/multichannel-probe-module.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("MultichannelProbe");

NS_OBJECT_ENSURE_REGISTERED (MultichannelProbe);

TypeId
MultichannelProbe::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::MultichannelProbe")
    .SetParent<Object> ()
    .SetGroupName("MultichannelProbe")
    .AddAttribute ("Interval",
                   "Measurement interval.",
                   TimeValue (Time ("100ms")),
                   MakeTimeAccessor (&MultichannelProbe::m_interval),
                   MakeTimeChecker ())
  ;
  return tid;
}

MultichannelProbe::MultichannelProbe (const std::string &fileName) :
  m_enabled (true),
  m_nonzero (false),
  m_header_written (false),
  m_fileName (fileName)
{
  NS_LOG_FUNCTION (this);

  m_file.open(m_fileName);

  // schedule update event
  m_updateEvent = Simulator::ScheduleNow (&MultichannelProbe::Update, this);
}

MultichannelProbe::~MultichannelProbe ()
{
  NS_LOG_FUNCTION (this);
}

void
MultichannelProbe::Enable (void)
{
  NS_LOG_FUNCTION (this);

  m_enabled = true;

  if (!m_updateEvent.IsRunning())
    {
      m_updateEvent = Simulator::ScheduleNow (&MultichannelProbe::Update, this);
    }
}

void
MultichannelProbe::Disable (void)
{
  NS_LOG_FUNCTION (this);

  m_enabled = false;

  m_updateEvent.Cancel ();
}

bool
MultichannelProbe::GetEnabled (void) const
{
  NS_LOG_FUNCTION (this);

  return m_enabled;
}

void
MultichannelProbe::Start (const Time& startTime)
{
  NS_LOG_FUNCTION (this << startTime);

  m_startEvent = Simulator::Schedule (startTime,
                                      &MultichannelProbe::Enable, this);
}

void
MultichannelProbe::Stop (const Time& stopTime)
{
  NS_LOG_FUNCTION (this << stopTime);

  m_stopEvent = Simulator::Schedule (stopTime,
                                     &MultichannelProbe::Disable, this);
}

void
MultichannelProbe::Update (void)
{
  NS_LOG_FUNCTION (this);

  bool new_nonzero = false;

  // write header if necessary
  if (!m_header_written)
    {
      m_file << "Time";

      for (uint32_t i = 0; i < m_probes.size (); ++i)
        {
          if (m_probes[i].queue)
            {
              m_file << "," << m_probes[i].identifier << ".EnqueueBytes";
              m_file << "," << m_probes[i].identifier << ".EnqueuePackets";
              m_file << "," << m_probes[i].identifier << ".DropBytes";
              m_file << "," << m_probes[i].identifier << ".DropPackets";
              m_file << "," << m_probes[i].identifier << ".DequeueBytes";
              m_file << "," << m_probes[i].identifier << ".DequeuePackets";
              m_file << "," << m_probes[i].identifier << ".OccupancyBytes";
              m_file << "," << m_probes[i].identifier << ".OccupancyPackets";
              m_file << "," << m_probes[i].identifier << ".MinOccupancyBytes";
              m_file << "," << m_probes[i].identifier << ".MinOccupancyPackets";
              m_file << "," << m_probes[i].identifier << ".MaxOccupancyBytes";
              m_file << "," << m_probes[i].identifier << ".MaxOccupancyPackets";
            }
          else if (m_probes[i].dev)
            {
              m_file << "," << m_probes[i].identifier << ".TxBytes";
              m_file << "," << m_probes[i].identifier << ".TxPackets";
              m_file << "," << m_probes[i].identifier << ".TxDropBytes";
              m_file << "," << m_probes[i].identifier << ".TxDropPackets";
              m_file << "," << m_probes[i].identifier << ".RxBytes";
              m_file << "," << m_probes[i].identifier << ".RxPackets";
            }
          else
            {
              m_file << "," << m_probes[i].identifier << ".Bytes";
              m_file << "," << m_probes[i].identifier << ".Packets";
            }
        }

      m_file << std::endl;

      m_header_written = true;
    }

  // dump and reset counters
  if (m_nonzero)
    {
      m_file << Simulator::Now ().GetSeconds ();
    }

  for (uint32_t i = 0; i < m_probes.size (); ++i)
    {
      if (m_probes[i].queue)
        {
          if (m_nonzero)
            {
              m_file << "," << m_probes[i].tx_enqueue_bytes;
              m_file << "," << m_probes[i].tx_enqueue_packets;
              m_file << "," << m_probes[i].tx_enqueue_drop_bytes;
              m_file << "," << m_probes[i].tx_enqueue_drop_packets;
              m_file << "," << m_probes[i].rx_dequeue_bytes;
              m_file << "," << m_probes[i].rx_dequeue_packets;
              m_file << "," << m_probes[i].occupancy_bytes;
              m_file << "," << m_probes[i].occupancy_packets;
              m_file << "," << m_probes[i].min_occupancy_bytes;
              m_file << "," << m_probes[i].min_occupancy_packets;
              m_file << "," << m_probes[i].max_occupancy_bytes;
              m_file << "," << m_probes[i].max_occupancy_packets;
            }

          m_probes[i].tx_enqueue_bytes = 0;
          m_probes[i].tx_enqueue_packets = 0;
          m_probes[i].tx_enqueue_drop_bytes = 0;
          m_probes[i].tx_enqueue_drop_packets = 0;
          m_probes[i].rx_dequeue_bytes = 0;
          m_probes[i].rx_dequeue_packets = 0;

          m_probes[i].min_occupancy_bytes = m_probes[i].occupancy_bytes;
          m_probes[i].max_occupancy_bytes = m_probes[i].occupancy_bytes;
          m_probes[i].min_occupancy_packets = m_probes[i].occupancy_packets;
          m_probes[i].max_occupancy_packets = m_probes[i].occupancy_packets;

          new_nonzero |= (m_probes[i].occupancy_bytes > 0) || (m_probes[i].occupancy_packets > 0);
        }
      else if (m_probes[i].dev)
        {
          if (m_nonzero)
            {
              m_file << "," << m_probes[i].tx_enqueue_bytes;
              m_file << "," << m_probes[i].tx_enqueue_packets;
              m_file << "," << m_probes[i].tx_enqueue_drop_bytes;
              m_file << "," << m_probes[i].tx_enqueue_drop_packets;
              m_file << "," << m_probes[i].rx_dequeue_bytes;
              m_file << "," << m_probes[i].rx_dequeue_packets;
            }

          m_probes[i].tx_enqueue_bytes = 0;
          m_probes[i].tx_enqueue_packets = 0;
          m_probes[i].tx_enqueue_drop_bytes = 0;
          m_probes[i].tx_enqueue_drop_packets = 0;
          m_probes[i].rx_dequeue_bytes = 0;
          m_probes[i].rx_dequeue_packets = 0;
        }
      else
        {
          if (m_nonzero)
            {
              m_file << "," << m_probes[i].tx_enqueue_bytes;
              m_file << "," << m_probes[i].tx_enqueue_packets;
            }

          m_probes[i].tx_enqueue_bytes = 0;
          m_probes[i].tx_enqueue_packets = 0;
        }
    }

  if (m_nonzero)
    {
      m_file << std::endl;
    }

  m_nonzero = new_nonzero;

  // schedule update event
  m_updateEvent = Simulator::Schedule (m_interval, &MultichannelProbe::Update, this);
}

bool
MultichannelProbe::AttachPacketTraceSource (Ptr<Object> obj, const std::string &probeTraceSource, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << obj << probeTraceSource << identifier);

  bool res = false;

  struct ProbeStats ps;
  ps.dev = false;
  ps.queue = false;
  ps.drop_was_enqueued = false;
  ps.explicit_occupancy = false;

  ps.tx_enqueue_bytes = 0;
  ps.tx_enqueue_packets = 0;
  ps.tx_enqueue_drop_bytes = 0;
  ps.tx_enqueue_drop_packets = 0;
  ps.rx_dequeue_bytes = 0;
  ps.rx_dequeue_packets = 0;
  ps.occupancy_bytes = 0;
  ps.occupancy_packets = 0;
  ps.min_occupancy_bytes = 0;
  ps.min_occupancy_packets = 0;
  ps.max_occupancy_bytes = 0;
  ps.max_occupancy_packets = 0;

  ps.identifier = identifier;

  if (obj->TraceConnect(probeTraceSource, std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketTraceSink, this)))
    {
      res = true;
    }

  if (res)
    {
      m_probes.push_back(ps);
    }

  return res;
}

bool
MultichannelProbe::AttachAll ()
{
  NS_LOG_FUNCTION (this);

  bool res = NodeList::GetNNodes () > 0;
  for (NodeList::Iterator i = NodeList::Begin (); i != NodeList::End (); ++i)
    {
      res = res & Attach (*i, "");
    }
  return res;
}

bool 
MultichannelProbe::Attach (NodeContainer nodes, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << identifier);

  bool res = nodes.GetN () > 0;
  for (uint32_t i = 0; i < nodes.GetN (); ++i)
    {
      std::ostringstream oss;
      if (identifier.length () > 0)
        {
          oss << identifier << ".n" << i;
        }
      res = res & Attach (nodes.Get (i), oss.str ());
    }
  return res;
}

bool 
MultichannelProbe::Attach (Ptr<Node> node, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << node << identifier);

  bool res = node->GetNDevices () > 0;
  for (uint32_t i = 0; i < node->GetNDevices (); ++i)
    {
      std::ostringstream oss;
      if (identifier.length () > 0)
        {
          oss << identifier << ".d" << i;
        }
      res = res & Attach (node->GetDevice (i), oss.str ());
    }
  return res;
}

bool 
MultichannelProbe::Attach (NetDeviceContainer devs, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << identifier);

  bool res = devs.GetN () > 0;
  for (uint32_t i = 0; i < devs.GetN (); ++i)
    {
      std::ostringstream oss;
      if (identifier.length () > 0)
        {
          oss << identifier << ".d" << i;
        }
      res = res & Attach (devs.Get (i), oss.str ());
    }
  return res;
}

bool 
MultichannelProbe::Attach (Ptr<NetDevice> dev, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << dev << identifier);

  bool res = false;

  struct ProbeStats ps;
  ps.dev = true;
  ps.queue = false;
  ps.drop_was_enqueued = false;
  ps.explicit_occupancy = false;

  ps.tx_enqueue_bytes = 0;
  ps.tx_enqueue_packets = 0;
  ps.tx_enqueue_drop_bytes = 0;
  ps.tx_enqueue_drop_packets = 0;
  ps.rx_dequeue_bytes = 0;
  ps.rx_dequeue_packets = 0;
  ps.occupancy_bytes = 0;
  ps.occupancy_packets = 0;
  ps.min_occupancy_bytes = 0;
  ps.min_occupancy_packets = 0;
  ps.max_occupancy_bytes = 0;
  ps.max_occupancy_packets = 0;

  if (identifier.length () > 0)
    {
      ps.identifier = identifier;
    }
  else
    {
      std::ostringstream oss;
      oss << "n" << dev->GetNode ()->GetId ();
      oss << ".d" << dev->GetIfIndex ();
      ps.identifier = oss.str();
    }

  // connect one TX trace source
  if (dev->TraceConnect("Tx", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketTxEnqueueTraceSink, this)))
    {
      res = true;
    }
  else if (dev->TraceConnect("PhyTxBegin", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketTxEnqueueTraceSink, this)))
    {
      res = true;
    }
  else if (dev->TraceConnect("PhyTxEnd", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketTxEnqueueTraceSink, this)))
    {
      res = true;
    }
  else if (dev->TraceConnect("MacTx", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketTxEnqueueTraceSink, this)))
    {
      res = true;
    }

  // connect all drop trace sources
  if (dev->TraceConnect("Drop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketDropTraceSink, this)))
    {
      res = true;
    }

  if (dev->TraceConnect("TxDrop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketDropTraceSink, this)))
    {
      res = true;
    }

  if (dev->TraceConnect("PhyTxDrop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketDropTraceSink, this)))
    {
      res = true;
    }

  if (dev->TraceConnect("MacTxDrop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketDropTraceSink, this)))
    {
      res = true;
    }

  // connect one RX trace source
  if (dev->TraceConnect("Rx", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketRxDequeueTraceSink, this)))
    {
      res = true;
    }
  else if (dev->TraceConnect("PhyRxEnd", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketRxDequeueTraceSink, this)))
    {
      res = true;
    }
  else if (dev->TraceConnect("PhyRxBegin", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketRxDequeueTraceSink, this)))
    {
      res = true;
    }
  else if (dev->TraceConnect("MacRx", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketRxDequeueTraceSink, this)))
    {
      res = true;
    }

  if (res)
    {
      m_probes.push_back(ps);

      // queues
      PointerValue ptr;
      dev->GetAttributeFailSafe("TxQueue", ptr);
      Ptr<Queue<QueueItem>> queue = ptr.Get<Queue<QueueItem>> ();

      if (queue)
        {
          std::ostringstream oss;
          oss << ps.identifier << ".txq";
          Attach(queue, oss.str ());
        }

      // queue discs
      Ptr<TrafficControlLayer> tc = dev->GetNode ()->GetObject<TrafficControlLayer> ();
      if (tc)
        {
          Ptr<QueueDisc> qd = DynamicCast<QueueDisc> (tc->GetRootQueueDiscOnDevice (dev));
          if (qd)
            {
              std::ostringstream oss;
              oss << ps.identifier << ".rqd";
              Attach(qd, oss.str ());
            }
        }
    }

  return res;
}

bool 
MultichannelProbe::Attach (Ptr<QueueDisc> qd, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << qd << identifier);

  bool res = false;

  struct ProbeStats ps;
  ps.dev = false;
  ps.queue = true;
  ps.drop_was_enqueued = true;
  ps.explicit_occupancy = false;

  ps.tx_enqueue_bytes = 0;
  ps.tx_enqueue_packets = 0;
  ps.tx_enqueue_drop_bytes = 0;
  ps.tx_enqueue_drop_packets = 0;
  ps.rx_dequeue_bytes = 0;
  ps.rx_dequeue_packets = 0;
  ps.occupancy_bytes = 0;
  ps.occupancy_packets = 0;
  ps.min_occupancy_bytes = 0;
  ps.min_occupancy_packets = 0;
  ps.max_occupancy_bytes = 0;
  ps.max_occupancy_packets = 0;

  ps.identifier = identifier;

  if (qd->TraceConnect("Enqueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::QueueDiscItemTxEnqueueTraceSink, this)))
    {
      res = true;
    }

  if (qd->TraceConnect("Drop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::QueueDiscItemDropTraceSink, this)))
    {
      res = true;
    }

  if (qd->TraceConnect("Dequeue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::QueueDiscItemRxDequeueTraceSink, this)))
    {
      res = true;
    }

  // if (qd->TraceConnect("BytesInQueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::OccupancyBytesTraceSink, this)))
  //   {
  //     res = true;
  //     ps.explicit_occupancy = true;
  //   }

  // if (qd->TraceConnect("PacketsInQueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::OccupancyPacketsTraceSink, this)))
  //   {
  //     res = true;
  //     ps.explicit_occupancy = true;
  //   }

  if (res)
    {
      m_probes.push_back(ps);
    }

  for (uint32_t i = 0; i < qd->GetNInternalQueues (); ++i)
    {
      std::ostringstream oss;
      if (identifier.length () > 0)
        {
          oss << identifier << ".q" << i;
        }
      res = res & Attach (qd->GetInternalQueue (i), oss.str ());
    }
  return res;
}

bool 
MultichannelProbe::Attach (Ptr<Queue<QueueItem>> queue, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << queue << identifier);

  bool res = false;

  struct ProbeStats ps;
  ps.dev = false;
  ps.queue = true;
  ps.drop_was_enqueued = false;
  ps.explicit_occupancy = false;

  ps.tx_enqueue_bytes = 0;
  ps.tx_enqueue_packets = 0;
  ps.tx_enqueue_drop_bytes = 0;
  ps.tx_enqueue_drop_packets = 0;
  ps.rx_dequeue_bytes = 0;
  ps.rx_dequeue_packets = 0;
  ps.occupancy_bytes = 0;
  ps.occupancy_packets = 0;
  ps.min_occupancy_bytes = 0;
  ps.min_occupancy_packets = 0;
  ps.max_occupancy_bytes = 0;
  ps.max_occupancy_packets = 0;

  ps.identifier = identifier;

  if (queue->TraceConnect("Enqueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketTxEnqueueTraceSink, this)))
    {
      res = true;
    }

  if (queue->TraceConnect("Drop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketDropTraceSink, this)))
    {
      res = true;
    }

  if (queue->TraceConnect("Dequeue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::PacketRxDequeueTraceSink, this)))
    {
      res = true;
    }

  // if (queue->TraceConnect("BytesInQueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::OccupancyBytesTraceSink, this)))
  //   {
  //     res = true;
  //     ps.explicit_occupancy = true;
  //   }

  // if (queue->TraceConnect("PacketsInQueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::OccupancyPacketsTraceSink, this)))
  //   {
  //     res = true;
  //     ps.explicit_occupancy = true;
  //   }

  if (res)
    {
      m_probes.push_back(ps);
    }

  return res;
}

bool 
MultichannelProbe::Attach (Ptr<Queue<Packet>> queue, const std::string &identifier)
{
    printf("CAUGHT!");
    return true;
}

bool 
MultichannelProbe::Attach (Ptr<Queue<QueueDiscItem>> queue, const std::string &identifier)
{
  NS_LOG_FUNCTION (this << queue << identifier);

  bool res = false;

  struct ProbeStats ps;
  ps.dev = false;
  ps.queue = true;
  ps.drop_was_enqueued = false;
  ps.explicit_occupancy = false;

  ps.tx_enqueue_bytes = 0;
  ps.tx_enqueue_packets = 0;
  ps.tx_enqueue_drop_bytes = 0;
  ps.tx_enqueue_drop_packets = 0;
  ps.rx_dequeue_bytes = 0;
  ps.rx_dequeue_packets = 0;
  ps.occupancy_bytes = 0;
  ps.occupancy_packets = 0;
  ps.min_occupancy_bytes = 0;
  ps.min_occupancy_packets = 0;
  ps.max_occupancy_bytes = 0;
  ps.max_occupancy_packets = 0;

  ps.identifier = identifier;

  if (queue->TraceConnect("Enqueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::QueueDiscItemTxEnqueueTraceSink, this)))
    {
      res = true;
    }

  if (queue->TraceConnect("Drop", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::QueueDiscItemDropTraceSink, this)))
    {
      res = true;
    }

  if (queue->TraceConnect("Dequeue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::QueueDiscItemRxDequeueTraceSink, this)))
    {
      res = true;
    }

  // if (queue->TraceConnect("BytesInQueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::OccupancyBytesTraceSink, this)))
  //   {
  //     res = true;
  //     ps.explicit_occupancy = true;
  //   }

  // if (queue->TraceConnect("PacketsInQueue", std::to_string(m_probes.size()), MakeCallback(&MultichannelProbe::OccupancyPacketsTraceSink, this)))
  //   {
  //     res = true;
  //     ps.explicit_occupancy = true;
  //   }

  if (res)
    {
      m_probes.push_back(ps);
    }

  return res;
}

void
MultichannelProbe::PacketTraceSink (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << context << packet);

  uint32_t index = std::stoi(context);
  m_probes[index].tx_enqueue_bytes += packet->GetSize();
  m_probes[index].tx_enqueue_packets += 1;
  m_nonzero = true;
}

void
MultichannelProbe::PacketTxEnqueueTraceSink (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << context << packet);

  uint32_t index = std::stoi(context);
  m_probes[index].tx_enqueue_bytes += packet->GetSize();
  m_probes[index].tx_enqueue_packets += 1;

  if (m_probes[index].queue && !m_probes[index].explicit_occupancy)
    {
      m_probes[index].occupancy_bytes += packet->GetSize();
      m_probes[index].occupancy_packets += 1;

      if (m_probes[index].occupancy_bytes > m_probes[index].max_occupancy_bytes)
        {
          m_probes[index].max_occupancy_bytes = m_probes[index].occupancy_bytes;
        }

      if (m_probes[index].occupancy_packets > m_probes[index].max_occupancy_packets)
        {
          m_probes[index].max_occupancy_packets = m_probes[index].occupancy_packets;
        }
    }
  m_nonzero = true;
}

void
MultichannelProbe::PacketDropTraceSink (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << context << packet);

  uint32_t index = std::stoi(context);
  m_probes[index].tx_enqueue_drop_bytes += packet->GetSize();
  m_probes[index].tx_enqueue_drop_packets += 1;

  if (m_probes[index].queue && m_probes[index].drop_was_enqueued && !m_probes[index].explicit_occupancy)
    {
      m_probes[index].occupancy_bytes -= packet->GetSize();
      m_probes[index].occupancy_packets -= 1;

      if (m_probes[index].occupancy_bytes < m_probes[index].min_occupancy_bytes)
        {
          m_probes[index].min_occupancy_bytes = m_probes[index].occupancy_bytes;
        }

      if (m_probes[index].occupancy_packets < m_probes[index].min_occupancy_packets)
        {
          m_probes[index].min_occupancy_packets = m_probes[index].occupancy_packets;
        }
    }
  m_nonzero = true;
}

void
MultichannelProbe::PacketRxDequeueTraceSink (std::string context, Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (this << context << packet);

  uint32_t index = std::stoi(context);
  m_probes[index].rx_dequeue_bytes += packet->GetSize();
  m_probes[index].rx_dequeue_packets += 1;

  if (m_probes[index].queue && !m_probes[index].explicit_occupancy)
    {
      m_probes[index].occupancy_bytes -= packet->GetSize();
      m_probes[index].occupancy_packets -= 1;

      if (m_probes[index].occupancy_bytes < m_probes[index].min_occupancy_bytes)
        {
          m_probes[index].min_occupancy_bytes = m_probes[index].occupancy_bytes;
        }

      if (m_probes[index].occupancy_packets < m_probes[index].min_occupancy_packets)
        {
          m_probes[index].min_occupancy_packets = m_probes[index].occupancy_packets;
        }
    }
  m_nonzero = true;
}

void
MultichannelProbe::QueueItemTraceSink (std::string context, Ptr<const QueueItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketTraceSink(context, item->GetPacket ());
}

void
MultichannelProbe::QueueItemTxEnqueueTraceSink (std::string context, Ptr<const QueueItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketTxEnqueueTraceSink(context, item->GetPacket ());
}

void
MultichannelProbe::QueueItemDropTraceSink (std::string context, Ptr<const QueueItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketDropTraceSink(context, item->GetPacket ());
}

void
MultichannelProbe::QueueItemRxDequeueTraceSink (std::string context, Ptr<const QueueItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketRxDequeueTraceSink(context, item->GetPacket ());
}

////////////////////////////////////////DUPLICATE PROBES FOR QUEUEUE DISKS
void
MultichannelProbe::QueueDiscItemTraceSink (std::string context, Ptr<const QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketTraceSink(context, item->GetPacket ());
}

void
MultichannelProbe::QueueDiscItemTxEnqueueTraceSink (std::string context, Ptr<const QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketTxEnqueueTraceSink(context, item->GetPacket ());
}

void
MultichannelProbe::QueueDiscItemDropTraceSink (std::string context, Ptr<const QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketDropTraceSink(context, item->GetPacket ());
}

void
MultichannelProbe::QueueDiscItemRxDequeueTraceSink (std::string context, Ptr<const QueueDiscItem> item)
{
  NS_LOG_FUNCTION (this << context << item);

  PacketRxDequeueTraceSink(context, item->GetPacket ());
}
//\\\\\\\\//////////////////////////////////////DUPLICATE PROBES FOR QUEUEUE DISKS

void
MultichannelProbe::OccupancyBytesTraceSink (std::string context, uint32_t old_bytes, uint32_t new_bytes)
{
  NS_LOG_FUNCTION (this << context << old_bytes << new_bytes);

  uint32_t index = std::stoi(context);
  m_probes[index].occupancy_bytes = new_bytes;

  if (new_bytes > m_probes[index].max_occupancy_bytes)
    {
      m_probes[index].max_occupancy_bytes = new_bytes;
    }

  if (new_bytes < m_probes[index].min_occupancy_bytes)
    {
      m_probes[index].min_occupancy_bytes = new_bytes;
    }
}

void
MultichannelProbe::OccupancyPacketsTraceSink (std::string context, uint32_t old_packets, uint32_t new_packets)
{
  NS_LOG_FUNCTION (this << context << old_packets << new_packets);

  uint32_t index = std::stoi(context);
  m_probes[index].occupancy_packets = new_packets;

  if (new_packets > m_probes[index].max_occupancy_packets)
    {
      m_probes[index].max_occupancy_packets = new_packets;
    }

  if (new_packets < m_probes[index].min_occupancy_packets)
    {
      m_probes[index].min_occupancy_packets = new_packets;
    }
}

} // Namespace ns3

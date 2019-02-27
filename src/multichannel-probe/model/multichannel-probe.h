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

#ifndef MULTICHANNEL_PROBE_H
#define MULTICHANNEL_PROBE_H

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/traffic-control-module.h"

namespace ns3 {

/**
 * \ingroup multichannel-probe
 * \defgroup multichannel-probe MultichannelProbe
 *
 * This 
 */

/**
 * \ingroup multichannel-probe
 *
 * \brief 
 *
 * This 
 *
 */
class MultichannelProbe : public Object
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);

  MultichannelProbe (const std::string &fileName);

  virtual ~MultichannelProbe ();

  bool GetEnabled () const;

  void Enable ();

  void Disable ();

  virtual void Start (const Time& startTime);

  virtual void Stop (const Time& stopTime);

  bool AttachPacketTraceSource (Ptr<Object> obj, const std::string &probeTraceSource, const std::string &identifier);
  bool AttachAll ();
  bool Attach (NodeContainer nodes, const std::string &identifier = "");
  bool Attach (Ptr<Node> node, const std::string &identifier = "");
  bool Attach (NetDeviceContainer devs, const std::string &identifier = "");
  bool Attach (Ptr<NetDevice> dev, const std::string &identifier = "");
  bool Attach (Ptr<QueueDisc> qd, const std::string &identifier = "");
  bool Attach (Ptr<Queue<Packet>> queue, const std::string &identifier = "");
  bool Attach (Ptr<Queue<QueueItem>> queue, const std::string &identifier = "");
  bool Attach (Ptr<Queue<QueueDiscItem>> queue, const std::string &identifier = "");

private:
  void Update (void);

  void PacketTraceSink (std::string context, Ptr<const Packet> packet);
  void PacketTxEnqueueTraceSink (std::string context, Ptr<const Packet> packet);
  void PacketDropTraceSink (std::string context, Ptr<const Packet> packet);
  void PacketRxDequeueTraceSink (std::string context, Ptr<const Packet> packet);

  void QueueItemTraceSink (std::string context, Ptr<const QueueItem> item);
  void QueueItemTxEnqueueTraceSink (std::string context, Ptr<const QueueItem> item);
  void QueueItemDropTraceSink (std::string context, Ptr<const QueueItem> item);
  void QueueItemRxDequeueTraceSink (std::string context, Ptr<const QueueItem> item);


    void QueueDiscItemTraceSink (std::string context, Ptr<const QueueDiscItem> item);
    void QueueDiscItemTxEnqueueTraceSink (std::string context, Ptr<const QueueDiscItem> item);
    void QueueDiscItemDropTraceSink (std::string context, Ptr<const QueueDiscItem> item);
    void QueueDiscItemRxDequeueTraceSink (std::string context, Ptr<const QueueDiscItem> item);


  void OccupancyBytesTraceSink (std::string context, uint32_t old_bytes, uint32_t new_bytes);
  void OccupancyPacketsTraceSink (std::string context, uint32_t old_packets, uint32_t new_packets);

  bool m_enabled;
  Time m_interval;
  bool m_nonzero;

  bool m_header_written;
  std::string m_fileName;
  std::ofstream m_file;

  EventId m_startEvent;
  EventId m_stopEvent;
  EventId m_updateEvent;

  struct ProbeStats
  {
    std::string identifier;
    bool dev;
    bool queue;
    bool drop_was_enqueued;
    bool explicit_occupancy;
    uint64_t tx_enqueue_bytes;
    uint64_t tx_enqueue_packets;
    uint64_t tx_enqueue_drop_bytes;
    uint64_t tx_enqueue_drop_packets;
    uint64_t rx_dequeue_bytes;
    uint64_t rx_dequeue_packets;
    uint64_t occupancy_bytes;
    uint64_t occupancy_packets;
    uint64_t min_occupancy_bytes;
    uint64_t min_occupancy_packets;
    uint64_t max_occupancy_bytes;
    uint64_t max_occupancy_packets;
  };

  std::vector<struct ProbeStats> m_probes;
};

} // namespace ns3

#endif /* MULTICHANNEL_PROBE_H */

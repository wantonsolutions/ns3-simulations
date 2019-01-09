
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

#ifndef RAID_H
#define RAID_H

#define RAID_REQUEST_SIZE 4096
#define RAID_COMPLETE 1
#define RAID_FIXABLE 2
#define RAID_INCOMPLETE 3

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/socket.h"

namespace ns3 {

struct RaidState {
  bool **Served_Raid_Requests;
  Ptr<Packet> **Served_Raid_Packets;
};

RaidState* InitRaidState(int parallel);
Ptr<Packet> RaidReceive(Ptr<Packet> packet, Address from, RaidState *rs, int parallel);
void RaidWrite(Ptr<Packet>* packets, Ptr<Socket> socket, Address to, int parallel);
int GetHitIndex(Address from, int requestIndex);
Ptr<Packet>* StripePacket(int parallel, uint32_t size, uint32_t sent, uint8_t* data);
int GetRaidFlowState(int requestIndex, int parallel, RaidState *rs);
Ptr<Packet> FixPacket(int requestIndex, int parallel, RaidState *rs);
Ptr<Packet> MergePacket(int requestIndex, int parallel, RaidState *rs);


} // namespace ns3

#endif /* RAID_CLIENT_H */

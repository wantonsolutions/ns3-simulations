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

#ifndef D_REDUNDANCY_SERVER_H
#define D_REDUNDANCY_SERVER_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"

#define SERVICE_BUFFER_SIZE 16777216
namespace ns3 {

class Socket;
class Packet;

/**
 * \ingroup applications 
 * \defgroup dredundancy DRedundancy
 */

/**
 * \ingroup dredundancy
 * \brief A D Redundancy server
 *
 * Every packet received is sent back.
 */
class DRedundancyServer : public Application 
{
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId (void);
  DRedundancyServer ();
  virtual ~DRedundancyServer ();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  void HandleRead (Ptr<Socket> socket);
  void VerboseServerSendPrint(Address from, Ptr<Packet> packet);
  void VerboseServerReceivePrint(Address from, Ptr<Packet> packet);

  void BroadcastWrite(Ptr<Packet> packet, Ptr<Socket> socket, Address from);
  void PrintSocketIP( Ptr<Socket> socket);

  /*
   * connect to a socket and return it
   */
  Ptr<Socket> ConnectSocket(uint16_t port, Ptr<NetDevice> dev);

  uint16_t m_port; //!< Port on which we listen for incoming packets.
  Ptr<Socket> m_socket; //!< IPv4 Socket
  Ptr<Socket> m_socket6; //!< IPv6 Socket

  Ptr<Socket>* m_sockets;
  Address m_local; //!< local multicast address //Todo get multiple addresses

  uint8_t m_parallel;

//Structs used to keep track of client requests
	bool m_served_requests[SERVICE_BUFFER_SIZE];
	int m_min, m_max;


};

} // namespace ns3

#endif /* D_REDUNDANCY_SERVER_H */


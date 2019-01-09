/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "raid-helper.h"
#include "ns3/raid-server.h"
#include "ns3/raid-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

RaidServerHelper::RaidServerHelper (uint16_t port, Address* ips, uint8_t parallel)
{
  m_factory.SetTypeId (RaidServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("Parallel", UintegerValue (parallel));
}

void 
RaidServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
RaidServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
RaidServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
RaidServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
RaidServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<RaidServer> ();
  node->AddApplication (app);

  return app;
}

RaidClientHelper::RaidClientHelper (uint16_t port, Address* addresses, uint8_t parallel)
{
  //Because I am being forced to use such a narrow interface to manipulate the underlying client, I think that before.
  //RaidClient drc = RaidClient::GetTypeId();
  //drc.SetAddresses(addresses,paralell);
  m_factory.SetTypeId (RaidClient::GetTypeId());
  SetAttribute ("RemoteAddress", AddressValue (addresses[0]));
  SetAttribute ("Parallel", UintegerValue (parallel));
  SetAttribute ("RemotePort", UintegerValue (port));
}

RaidClientHelper::RaidClientHelper (Address address)
{
  m_factory.SetTypeId (RaidClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
RaidClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
RaidClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<RaidClient>()->SetFill (fill);
}

void
RaidClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<RaidClient>()->SetFill (fill, dataLength);
}

void
RaidClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<RaidClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
RaidClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
RaidClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
RaidClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
RaidClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<RaidClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3

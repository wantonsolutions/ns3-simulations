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
#include "d-redundancy-helper.h"
#include "ns3/d-redundancy-server.h"
#include "ns3/d-redundancy-client.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

DRedundancyServerHelper::DRedundancyServerHelper (uint16_t port, Address* ips, uint8_t parallel)
{
  m_factory.SetTypeId (DRedundancyServer::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
  SetAttribute ("Parallel", UintegerValue (parallel));
}

void 
DRedundancyServerHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
DRedundancyServerHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DRedundancyServerHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DRedundancyServerHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
DRedundancyServerHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<DRedundancyServer> ();
  node->AddApplication (app);

  return app;
}

DRedundancyClientHelper::DRedundancyClientHelper (uint16_t port, Address* addresses, uint8_t parallel)
{
  //Because I am being forced to use such a narrow interface to manipulate the underlying client, I think that before.
  //DRedundancyClient drc = DRedundancyClient::GetTypeId();
  //drc.SetAddresses(addresses,paralell);
  m_factory.SetTypeId (DRedundancyClient::GetTypeId());
  SetAttribute ("RemoteAddress", AddressValue (addresses[0]));
  SetAttribute ("Parallel", UintegerValue (parallel));
  SetAttribute ("RemotePort", UintegerValue (port));
}

DRedundancyClientHelper::DRedundancyClientHelper (Address address)
{
  m_factory.SetTypeId (DRedundancyClient::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
DRedundancyClientHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
DRedundancyClientHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<DRedundancyClient>()->SetFill (fill);
}

void
DRedundancyClientHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<DRedundancyClient>()->SetFill (fill, dataLength);
}

void
DRedundancyClientHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<DRedundancyClient>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
DRedundancyClientHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DRedundancyClientHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
DRedundancyClientHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
DRedundancyClientHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<DRedundancyClient> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3

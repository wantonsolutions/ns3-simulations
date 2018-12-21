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
#include "udp-echo-report-helper.h"
#include "ns3/udp-echo-server-report.h"
#include "ns3/udp-echo-client-report.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

UdpEchoServerReportHelper::UdpEchoServerReportHelper (uint16_t port)
{
  m_factory.SetTypeId (UdpEchoServerReport::GetTypeId ());
  SetAttribute ("Port", UintegerValue (port));
}

void 
UdpEchoServerReportHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

ApplicationContainer
UdpEchoServerReportHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpEchoServerReportHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpEchoServerReportHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpEchoServerReportHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpEchoServerReport> ();
  node->AddApplication (app);

  return app;
}

UdpEchoClientReportHelper::UdpEchoClientReportHelper (Address address, uint16_t port)
{
  m_factory.SetTypeId (UdpEchoClientReport::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
  SetAttribute ("RemotePort", UintegerValue (port));
}

UdpEchoClientReportHelper::UdpEchoClientReportHelper (Address address)
{
  m_factory.SetTypeId (UdpEchoClientReport::GetTypeId ());
  SetAttribute ("RemoteAddress", AddressValue (address));
}

void 
UdpEchoClientReportHelper::SetAttribute (
  std::string name, 
  const AttributeValue &value)
{
  m_factory.Set (name, value);
}

void
UdpEchoClientReportHelper::SetFill (Ptr<Application> app, std::string fill)
{
  app->GetObject<UdpEchoClientReport>()->SetFill (fill);
}

void
UdpEchoClientReportHelper::SetFill (Ptr<Application> app, uint8_t fill, uint32_t dataLength)
{
  app->GetObject<UdpEchoClientReport>()->SetFill (fill, dataLength);
}

void
UdpEchoClientReportHelper::SetFill (Ptr<Application> app, uint8_t *fill, uint32_t fillLength, uint32_t dataLength)
{
  app->GetObject<UdpEchoClientReport>()->SetFill (fill, fillLength, dataLength);
}

ApplicationContainer
UdpEchoClientReportHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpEchoClientReportHelper::Install (std::string nodeName) const
{
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return ApplicationContainer (InstallPriv (node));
}

ApplicationContainer
UdpEchoClientReportHelper::Install (NodeContainer c) const
{
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

  return apps;
}

Ptr<Application>
UdpEchoClientReportHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<UdpEchoClientReport> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3

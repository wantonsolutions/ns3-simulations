
/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MiddleBox");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer leftNodes;
  leftNodes.Create (2);

  NodeContainer rightNodes;
  rightNodes.Add(leftNodes.Get(1)); // set the middle node as a switch?
  rightNodes.Create (1);

  PointToPointHelper leftPointToPoint;
  leftPointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  leftPointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer leftDevices;
  leftDevices = leftPointToPoint.Install (leftNodes);

  PointToPointHelper rightPointToPoint;
  rightPointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  rightPointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer rightDevices;
  rightDevices = rightPointToPoint.Install (rightNodes);

  InternetStackHelper stack;
  stack.Install (leftNodes);
  stack.Install (rightNodes.Get(1));

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer leftInterfaces = address.Assign (leftDevices);


  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer rightInterfaces = address.Assign (rightDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (rightNodes.Get (1));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (rightInterfaces.GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (leftNodes.Get (0));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

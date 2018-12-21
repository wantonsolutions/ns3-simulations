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

  NodeContainer bottomNodes;
  bottomNodes.Create (4);

  NodeContainer topNodes;
  topNodes.Create (1);

  NodeContainer nc_top2Bottom[4];
  NetDeviceContainer ndc_top2Bottom[4];

  nc_top2Bottom[0] = NodeContainer( topNodes.Get(0), bottomNodes.Get(0) );
  nc_top2Bottom[1] = NodeContainer( topNodes.Get(0), bottomNodes.Get(1) );
  nc_top2Bottom[2] = NodeContainer( topNodes.Get(0), bottomNodes.Get(2) );
  nc_top2Bottom[3] = NodeContainer( topNodes.Get(0), bottomNodes.Get(3) );


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  ndc_top2Bottom[0] = pointToPoint.Install(nc_top2Bottom[0]);
  ndc_top2Bottom[1] = pointToPoint.Install(nc_top2Bottom[1]);
  ndc_top2Bottom[2] = pointToPoint.Install(nc_top2Bottom[2]);
  ndc_top2Bottom[3] = pointToPoint.Install(nc_top2Bottom[3]);

  InternetStackHelper stack;
  stack.Install (bottomNodes);
  stack.Install (topNodes);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces[4];
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces[0] = address.Assign (ndc_top2Bottom[0]);
  address.SetBase ("10.1.2.0", "255.255.255.0");
  interfaces[1] = address.Assign (ndc_top2Bottom[1]);
  address.SetBase ("10.1.3.0", "255.255.255.0");
  interfaces[2] = address.Assign (ndc_top2Bottom[2]);
  address.SetBase ("10.1.4.0", "255.255.255.0");
  interfaces[3] = address.Assign (ndc_top2Bottom[3]);


  //address.SetBase ("10.1.2.0", "255.255.255.0");
  //Ipv4InterfaceContainer topInterfaces = address.Assign (topDevices);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (bottomNodes.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (interfaces[0].GetAddress (1), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (bottomNodes.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

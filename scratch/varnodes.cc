
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

NS_LOG_COMPONENT_DEFINE ("VarNodes");

const int NODES = 5;

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer bottomNodes;
  bottomNodes.Create (NODES);

  NodeContainer topNodes;
  topNodes.Create (1);

  NodeContainer nc_top2Bottom[NODES];
  NetDeviceContainer ndc_top2Bottom[NODES];
  
  for (int node = 0; node < NODES; node ++) {
      nc_top2Bottom[node] = NodeContainer( topNodes.Get(0), bottomNodes.Get(node) );
  }


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  for (int node = 0; node < NODES; node ++) {
    ndc_top2Bottom[node] = pointToPoint.Install(nc_top2Bottom[node]);
  }

  InternetStackHelper stack;
  stack.Install (bottomNodes);
  stack.Install (topNodes);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer interfaces[NODES];
  for (int node = 0; node < NODES; node ++) {
      std::stringstream addr;
      addr << "10.1" << node + 1 << ".1.0";
      address.SetBase (addr.str().c_str(), "255.255.255.0");
      interfaces[node] = address.Assign (ndc_top2Bottom[node]);
  }

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

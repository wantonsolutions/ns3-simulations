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

NS_LOG_COMPONENT_DEFINE ("VarClients");

const int SWITCHES = 2;
const int NODES = 2;
const int CLIENTS = 2;
const int SERVERS = 2;

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_DEBUG);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (NODES*SWITCHES);

  NodeContainer switches;
  switches.Create (SWITCHES);

  NodeContainer core;
  core.Create(1);

  NodeContainer nc_switch2node[NODES*SWITCHES];
  NetDeviceContainer ndc_switch2node[NODES*SWITCHES];

  NodeContainer nc_core2switch[SWITCHES];
  NetDeviceContainer ndc_core2switch[SWITCHES];
  
  for (int s = 0; s < SWITCHES; s++) {
      nc_core2switch[s] = NodeContainer(core.Get(0), switches.Get(s));
      //switches to node
      for (int node = 0; node < NODES; node ++) {
          nc_switch2node[(s*NODES) + node] = NodeContainer( switches.Get(s), nodes.Get((s*NODES) + node) );
      }
  }


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  for (int s = 0; s < SWITCHES; s++ ){
      //core to switch
      ndc_core2switch[s] = pointToPoint.Install(nc_core2switch[s]);
      //nodes to switches
      for (int node = 0; node < NODES; node ++) {
        ndc_switch2node[(s*NODES)+node] = pointToPoint.Install(nc_switch2node[(s*NODES)+ node]);
      }
  }

  InternetStackHelper stack;
  stack.Install (nodes);
  stack.Install (switches);
  stack.Install (core);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer node_interfaces[NODES*SWITCHES];
  Ipv4InterfaceContainer switch_interfaces[SWITCHES];
  for (int s = 0; s < SWITCHES; s++) {
      std::stringstream saddr;
      saddr << "10." << s+1 << ".0.0";
      address.SetBase(saddr.str().c_str(), "255.255.0.0");
      switch_interfaces[s] = address.Assign(ndc_core2switch[s]);

      for (int node = 0; node < NODES; node ++) {
          std::stringstream addr;
          addr << "10."<< s + 1 << "." << node + 1 << ".0";
          address.SetBase (addr.str().c_str(), "255.255.255.0");
          node_interfaces[(s*NODES) + node] = address.Assign (ndc_switch2node[s* NODES +node]);
      }
  }

  //address.SetBase ("10.1.2.0", "255.255.255.0");
  //Ipv4InterfaceContainer topInterfaces = address.Assign (topDevices);

  UdpEchoServerHelper echoServer (9);

  for (int server = 0; server < SERVERS; server ++ ){
      ApplicationContainer serverApps = echoServer.Install (nodes.Get (server));
      serverApps.Start (Seconds (1.0));
      serverApps.Stop (Seconds (10.0));
  }

  //UdpEchoClientHelper echoClient (interfaces[0].GetAddress (1), 9);
  //echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  //echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  //echoClient.SetAttribute ("PacketSize", UintegerValue (1024));
  
  for (int client = 0; client < CLIENTS; client++) {
      //map clients to servers 
      UdpEchoClientHelper echoClient (node_interfaces[client%SERVERS].GetAddress (1), 9);
      echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
      echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
      echoClient.SetAttribute ("PacketSize", UintegerValue (1024));


      ApplicationContainer clientApps = echoClient.Install (nodes.Get ((SERVERS)+client));
      clientApps.Start (Seconds (2.0));
      clientApps.Stop (Seconds (10.0));
  }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

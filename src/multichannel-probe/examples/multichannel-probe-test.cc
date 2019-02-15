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

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/multichannel-probe-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("multichannel-probe-test");

int 
main (int argc, char *argv[])
{
  
  CommandLine cmd;
  cmd.Parse (argc, argv);

  NS_LOG_INFO ("Create nodes.");

  NodeContainer sw_nodes;
  sw_nodes.Create (1);

  NodeContainer host_nodes;
  host_nodes.Create (4);

  InternetStackHelper internet;
  internet.Install (host_nodes);
  internet.Install (sw_nodes);

  NS_LOG_INFO ("Create channels.");

  NetDeviceContainer sw_devs;
  NetDeviceContainer host_devs;

  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
  p2p.SetChannelAttribute ("Delay", StringValue ("1us"));

  for (uint32_t i = 0; i < host_nodes.GetN (); ++i)
    {
      NetDeviceContainer nd = p2p.Install (sw_nodes.Get (0), host_nodes.Get (i));
      sw_devs.Add (nd.Get (0));
      host_devs.Add (nd.Get (1));
    }

  NS_LOG_INFO ("Assign IP addresses.");

  Ipv4AddressHelper host_address;
  host_address.SetBase ("10.0.0.0", "255.255.0.0");
  Ipv4InterfaceContainer host_if = host_address.Assign (host_devs);

  Ipv4AddressHelper sw_address;
  sw_address.SetBase ("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer sw_if = sw_address.Assign (sw_devs);

  NS_LOG_INFO ("Set up routing.");

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  NS_LOG_INFO ("Create sources.");

  uint16_t sinkPort = 8080;
  Address sinkAddress (InetSocketAddress (Ipv4Address::GetBroadcast (), sinkPort));

  OnOffHelper onOffHelper ("ns3::UdpSocketFactory", sinkAddress);
  //onOffHelper.SetConstantRate (DataRate("10Mbps"), 1470);
  onOffHelper.SetAttribute ("DataRate", StringValue ("10Mbps"));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (1470));
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::UniformRandomVariable[Min=0.1|Max=0.5]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::UniformRandomVariable[Min=0.1|Max=0.5]"));
  ApplicationContainer sourceApps;

  for (uint32_t i = 0; i < host_nodes.GetN (); ++i)
  {
    for (uint32_t j = 0; j < host_nodes.GetN (); ++j)
      {
        if (i != j)
          {
            onOffHelper.SetAttribute ("Remote", AddressValue (InetSocketAddress (host_nodes.Get (i)->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal (), sinkPort)));
            sourceApps.Add (onOffHelper.Install (host_nodes.Get (j)));
          }
      }
  }

  sourceApps.Start (Seconds (1.));
  sourceApps.Stop (Seconds (9.));

  NS_LOG_INFO ("Create sinks.");

  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
  ApplicationContainer sinkApps = packetSinkHelper.Install (host_nodes);
  sinkApps.Start (Seconds (0.));
  sinkApps.Stop (Seconds (10.));

  p2p.EnablePcapAll ("multichannel-probe-test");

  Ptr<MultichannelProbe> mcp = CreateObject<MultichannelProbe> ("multichannel-probe-test.csv");
  mcp->SetAttribute ("Interval", StringValue("10ms"));
  mcp->AttachAll ();

  Simulator::Stop (Seconds (20));
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}


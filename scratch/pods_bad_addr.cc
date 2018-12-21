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


const int K = 4;
const int PODS = K;
const int EDGE = (K/2);
const int EDGES = PODS * EDGE;
const int AGG = (K/2);
const int AGGS = PODS * AGG;
const int CORE = (K/2)*(K/2);
const int NODE = K/2 ;
const int NODES = PODS * EDGE * NODE ;

//Helper functions from the web
void printRoutingTable (Ptr<Node> node)
{
    Ipv4StaticRoutingHelper helper;
    Ptr<Ipv4> stack = node -> GetObject<Ipv4>();
    Ptr<Ipv4StaticRouting> staticrouting = helper.GetStaticRouting(stack);
    uint32_t numroutes=staticrouting->GetNRoutes();
    Ipv4RoutingTableEntry entry;
    std::cout << "Routing table for device: " << Names::FindName(node) <<"\n";
    std::cout << "Destination\tMask\t\tGateway\t\tIface\n";
    for (uint32_t i =0 ; i<numroutes;i++) {
        entry =staticrouting->GetRoute(i);
        std::cout << entry.GetDestNetwork()  << "\t" << entry.GetDestNetworkMask() << "\t" << entry.GetGateway() << "\t\t" << entry.GetInterface() << "\n";
     }
    return;
}

void setAddress(Ptr<NetDevice> nd, const char * addressString) {
    //printf("%s\n",addressString);

     Ptr<Node> node;
     Ptr<Ipv4> ipv4;
     Ipv4InterfaceAddress addr;
     Ipv4Address addressIp;
     const char * IPAddress;

     IPAddress = addressString;


     node = nd->GetNode();

     ipv4 = node->GetObject<Ipv4>(); // Get Ipv4 instance of the node

     int32_t interface = ipv4->GetInterfaceForDevice (nd);
     if (interface == -1) {
       interface = ipv4->AddInterface (nd);
     }

     Ipv4InterfaceAddress ipv4Addr = Ipv4InterfaceAddress (Ipv4Address(IPAddress), Ipv4Mask ("/0"));

     ipv4->AddAddress (interface, ipv4Addr);
     ipv4->SetMetric (interface, 1);
     ipv4->SetUp (interface);

     //TODO Start here: The problem is likely that I don't have interface tabels set up
}

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (NODES);

  NodeContainer edge;
  edge.Create(EDGES);

  NodeContainer agg;
  agg.Create(AGGS);

  NodeContainer core;
  core.Create(CORE);

  NodeContainer nc_node2edge[NODES];
  NetDeviceContainer ndc_node2edge[NODES];

  NodeContainer nc_edge2agg[EDGE * AGG * PODS];
  NetDeviceContainer ndc_edge2agg[EDGE * AGG * PODS];

  NodeContainer nc_agg2core[CORE*PODS];
  NetDeviceContainer ndc_agg2core[CORE*PODS];
  

  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      nc_node2edge[n] = NodeContainer(edge.Get(n/(K/2)), nodes.Get(n));
  }

  //connect edges to agg
  for (int pod = 0; pod < PODS; pod++) {
      for (int edgeS = 0; edgeS < EDGE; edgeS++) {
          for (int aggS = 0; aggS < AGG; aggS++) {
              int aggIndex = pod*AGG + aggS;
              int edgeIndex = pod*EDGE + edgeS;
              int link = (pod * (K/2)*(K/2)) + (edgeS * EDGE) + aggS;
              //int link = (pod * PODS) + (aggIndex) + edgeIndex;
              //printf("link %d, aggIndex %d, edgeIndex %d\n",link,aggIndex,edgeIndex);
              nc_edge2agg[link] = NodeContainer(agg.Get(aggIndex), edge.Get(edgeIndex));
          }
      }
  }

  //connect agg to core
  for (int coreS = 0; coreS < CORE;coreS++) {
      for (int pod = 0; pod < PODS; pod++) {
              nc_agg2core[(coreS * CORE) + pod] = NodeContainer(core.Get(coreS), agg.Get((pod*AGG) + (coreS/AGG)));
      }
  }


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));


  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      ndc_node2edge[n] = pointToPoint.Install(nc_node2edge[n]);
  }
  //connect edge to agg
  const int aggC = EDGE * AGG * PODS;
  //const int aggC = 1;
  for (int e=0;e<aggC;e++){
      //printf("edge %d total %d\n",e,aggC);
      //printf("c.GetN == %d\n",nc_edge2agg[e].GetN());
      ndc_edge2agg[e] = pointToPoint.Install(nc_edge2agg[e]);
  }
  //connect agg to cores
  for (int s = 0;s < CORE * PODS;s++) {
      ndc_agg2core[s] = pointToPoint.Install(nc_agg2core[s]);
  }


  InternetStackHelper stack;
  stack.Install (nodes);
  stack.Install (edge);
  stack.Install (agg);
  stack.Install (core);

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer node2edge[NODES];
  Ipv4InterfaceContainer edge2agg[EDGE*AGG*PODS];
  Ipv4InterfaceContainer agg2core[CORE*PODS];

  address.SetBase("10.1.1.0", "255.255.255.255");
  for (int i=0;i<NODES;i++) {
  	node2edge[i] = address.Assign(ndc_node2edge[i]);
  }
  for (int i=0;i<EDGE*AGG*PODS;i++) {
  	edge2agg[i] = address.Assign(ndc_edge2agg[i]);
  }
  for (int i=0;i<CORE*PODS;i++) {
  	agg2core[i] = address.Assign(ndc_agg2core[i]);
  }




  //Starting Here The question is how do I assign specific IP addresses rather than using the address.Assign counter which automatically assings addresses.
  int serverport = 9;
  int clientIndex = 0;
  int serverIndex = 11;
  UdpEchoServerHelper echoServer (serverport);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (serverIndex));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  //map clients to servers 
  UdpEchoClientHelper echoClient (node2edge[serverIndex].GetAddress(1), serverport);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (50000));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.001)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));


  ApplicationContainer clientApps = echoClient.Install (nodes.Get (clientIndex));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll("pods");
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("pods.tr"));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}



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
    printf("%s\n",addressString);

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
              printf("link %d, aggIndex %d, edgeIndex %d\n",link,aggIndex,edgeIndex);
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
      printf("edge %d total %d\n",e,aggC);
      printf("c.GetN == %d\n",nc_edge2agg[e].GetN());
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
  Ipv4InterfaceContainer node_interfaces[NODES];
  Ipv4InterfaceContainer edge_interfaces[EDGES];
  Ipv4InterfaceContainer agg_interfaces[AGGS];
  Ipv4InterfaceContainer core_interfaces[CORE];


  printf("%s\n","assigning addresses for nodes -> edge");
  for (int p = 0;p <PODS;p++) {
    for (int e = 0;e <EDGE;e++) {
      for(int n =0; n<NODE;n++) {
        std::stringstream nodeAddr;
        std::stringstream edgeAddr;
        int link = (p*PODS) + (e*EDGE) + n;
        printf("link %d\n",link);
	/*
        Ptr<NetDevice> ndn = ndc_node2edge[link].Get(0);
        Ptr<NetDevice> nde = ndc_node2edge[link].Get(1);
        edgeAddr << "10" << "." << p << "." << e << "." << 1;
        setAddress(ndn,nodeAddr.str().c_str());
        setAddress(nde,edgeAddr.str().c_str());
	*/
	//Second Try, lets use the address helper this time.
        //nodeAddr << "10" << "." << p << "." << e << "." << n+1;
	//address.SetBase(nodeAddr, "255.255.255.0")
	//After trying to loosen the constraints on IP I just ended up muttling the whole thing. THis file will sit here for now, and I'll get back to it later.
	
      }
    }
  }
    
  printf("%s\n","assigning addresses for edge and agg");
  for (int p = 0;p <PODS;p++) {
    for (int e = 0;e <EDGE;e++) {
        for (int a = 0;a <AGG;a++) {
        std::stringstream edgeAddr;
        std::stringstream aggrAddr;
        int link = (p*PODS) + (e * EDGE) + a;
        printf("link %d\n",link);
        Ptr<NetDevice> nde = ndc_edge2agg[link].Get(0);
        Ptr<NetDevice> nda = ndc_edge2agg[link].Get(1);
        edgeAddr << "10" << "." << p << "." << e << "." << 1;
        aggrAddr << "10" << "." << p << "." << a + EDGE << "." << 1;
        setAddress(nde,edgeAddr.str().c_str());
        setAddress(nda,aggrAddr.str().c_str());
        }
    }
  }

  //connect agg to core
  printf("%s\n","assigning addresses for agg and core");
  for (int c = 0; c < CORE; c++) {
      for (int p = 0; p < PODS; p++) {
              int link = (c * CORE) + p;
              int bin = c / (K/2);
              int binIndex = c % (K/2);
                printf("link -> %d\n",link);
                std::stringstream aggrAddr;
                std::stringstream coreAddr;
                Ptr<NetDevice> nda = ndc_agg2core[link].Get(0);
                Ptr<NetDevice> ndc = ndc_agg2core[link].Get(1);

                aggrAddr << "10" << "." << p << "." << bin + EDGE << "." << 1;
                coreAddr << "10" << "." << K << "." << bin + 1 << "." << binIndex + 1;
                setAddress(nda,aggrAddr.str().c_str());
                setAddress(ndc,coreAddr.str().c_str());

      }
  }


  //Starting Here The question is how do I assign specific IP addresses rather than using the address.Assign counter which automatically assings addresses.


  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (nodes.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  //map clients to servers 
  Ipv4Address ip = Ipv4Address("10.0.0.2");
  UdpEchoClientHelper echoClient (ip, 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (1));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));


  ApplicationContainer clientApps = echoClient.Install (nodes.Get (1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  for (int i =0;i<EDGES;i++) {
	printRoutingTable(edge.Get(i));
  }
  for (int i =0;i<NODES;i++) {
	printRoutingTable(nodes.Get(i));
  }

  pointToPoint.EnablePcapAll("pods");
  AsciiTraceHelper ascii;
  pointToPoint.EnableAsciiAll(ascii.CreateFileStream("pods.tr"));

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}



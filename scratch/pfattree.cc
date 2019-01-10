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
const int PARALLEL = 3;

const int PODS = K;
const int PERPOD = (K/2);
const int PPODS = PARALLEL * PODS;
const int CORE = (K/2)*(K/2);
const int PCORE = PARALLEL * CORE;
const int NODE = K/2 ;
const int NODES = PODS * PERPOD * NODE;


void InstallRandomClientTransmissions(float start, float stop, int clientIndex, UdpEchoClientHelper echoClient, NodeContainer nodes) {
  for (float base = start;base < stop; base += 1.0) {
	ApplicationContainer clientApps = echoClient.Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
	clientApps.Stop( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
  }
}

void InstallUniformClientTransmissions(float start, float stop, float gap, int clientIndex, UdpEchoClientHelper echoClient, NodeContainer nodes) {
  for (float base = start;base < stop; base += gap) {
  	ApplicationContainer clientApps = echoClient.Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += gap;
	clientApps.Stop( Seconds (base));
  }
}

void InstallClientAttributes(UdpEchoClientHelper echoClient, int maxpackets, double interval, int packetsize) {
  echoClient.SetAttribute ("MaxPackets", UintegerValue (maxpackets));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (interval)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (packetsize));
}

//Start with a test
void SetupRandomMeasureClient(float start, float stop, int serverPort, Address serverAddress, NodeContainer nodes, int clientIndex) {
  //map clients to servers 
  UdpEchoClientHelper echoClient (serverAddress, serverPort);
  InstallClientAttributes(echoClient,50000,0.1,20480);
  InstallRandomClientTransmissions(start,stop,clientIndex,echoClient,nodes);
}

void SetupModularClient(float start, float stop, float gap, int serverPort, Address serverAddress, NodeContainer nodes, int clientIndex, double interval, int packetsize) {
  //map clients to servers 
  UdpEchoClientHelper echoClient (serverAddress, serverPort);
  InstallClientAttributes(echoClient, 50000,interval,packetsize);
  InstallUniformClientTransmissions(start,stop,gap,clientIndex, echoClient, nodes);
}

void SetupModularRandomClient(float start, float stop, int serverPort, Address serverAddress, NodeContainer nodes, int clientIndex, double interval, int packetsize, int maxpackets) {
  //map clients to servers 
  UdpEchoClientHelper echoClient (serverAddress, serverPort);
  InstallClientAttributes(echoClient, maxpackets,interval,packetsize);
  InstallRandomClientTransmissions(start,stop,clientIndex,echoClient,nodes);
}

//Start with a test
void SetupUniformMeasureClient(float start, float stop, float gap, int serverPort, Address serverAddress, NodeContainer nodes, int clientIndex) {
  //map clients to servers 
  UdpEchoClientHelper echoClient (serverAddress, serverPort);
  InstallClientAttributes(echoClient,50000,0.001,1024);
  
  for (float base = start;base < stop; base += gap) {
  	ApplicationContainer clientApps = echoClient.Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += gap;
	clientApps.Stop( Seconds (base));
  }
}

void SetupRandomCoverTraffic(float clientStart,float clientStop, int NPackets, float interval, int packetsize, int serverport ,NodeContainer nodes, int numNodes, int distance, Ipv4InterfaceContainer addresses[]) {
  for (int i = 0; i < numNodes; i++) {
	  if ( ! bool(i % 2) ) {
		  UdpEchoServerHelper echoServer (serverport);
		  ApplicationContainer serverApps = echoServer.Install (nodes.Get (i));
		  serverApps.Start (Seconds (1.0));
		  serverApps.Stop (Seconds (clientStop));
	  } else {
		  //Pick the server in the nearist pod
                  int serverindex = ((i-1) + (distance)) % numNodes;
  	          Address serverAddress = addresses[serverindex].GetAddress(1);
		  //float interval =  static_cast <float> (rand()) / static_cast <float> (RAND_MAX) ;
		  //int packetsize = 1024 * (rand() % 32);
		  //printf("Starting Client %d with packetsize %d, rate %f\n",i/2,packetsize,interval); 
		  SetupModularRandomClient(clientStart,clientStop,serverport,serverAddress,nodes,i,interval,packetsize,NPackets);
	  }
  }
}

void SetupUniformCoverTraffic(float clientStart,float clientStop, float gap, float offset, int serverport ,NodeContainer nodes, int numNodes, int distance, Ipv4InterfaceContainer addresses[]) {

  for (int i = 0; i < numNodes; i++) {
	  if ( ! bool(i % 2) ) {
		  UdpEchoServerHelper echoServer (serverport);
		  ApplicationContainer serverApps = echoServer.Install (nodes.Get (i));
		  serverApps.Start (Seconds (1.0));
		  serverApps.Stop (Seconds (clientStop));
	  } else {
		  //Pick the server in the adjacent
          int serverindex = ((i-1) + (distance)) % numNodes;
  	      Address serverAddress = addresses[serverindex].GetAddress(1);
		  SetupUniformMeasureClient(clientStart,clientStop,gap,serverport,serverAddress,nodes,i);
	  }
  }
}



int
main (int argc, char *argv[])
{
  CommandLine cmd;

  uint32_t CoverNPackets;
  float CoverInterval;
  uint32_t CoverPacketSize;

  uint32_t ClientProtocolNPackets;
  float ClientProtocolInterval;
  uint32_t ClientProtocolPacketSize;


  cmd.AddValue("CoverNPackets", "Number of packets for the cover to echo", CoverNPackets);
  cmd.AddValue("CoverInterval", "Interval at which cover traffic broadcasts", CoverInterval);
  cmd.AddValue("CoverPacketSize", "The Size of the packet used by the cover traffic", CoverPacketSize);

  cmd.AddValue("ClientProtocolNPackets", "Number of packets to echo", ClientProtocolNPackets);
  cmd.AddValue("ClientProtocolInterval", "Interval at which a protocol client makes requests", ClientProtocolInterval);
  cmd.AddValue("ClientProtocolPacketSize", "Interval at which a protocol client makes requests", ClientProtocolPacketSize);

  cmd.Parse (argc, argv);

  printf("Client - NPackets %d, baseInterval %f packetSize %d \n",ClientProtocolNPackets,ClientProtocolInterval,ClientProtocolPacketSize);
  printf("Cover - NPackets %d, baseInterval %f packetSize %d \n",CoverNPackets,CoverInterval,CoverPacketSize);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("RaidClientApplication", LOG_LEVEL_WARN);
  //LogComponentEnable ("RaidClientApplication", LOG_LEVEL_INFO);
  //LogComponentEnable ("RaidServerApplication", LOG_LEVEL_INFO);

  NodeContainer nodes;
  nodes.Create (NODES);


  NodeContainer pods[PARALLEL];
  for (int i = 0;i<PARALLEL;i++) {
    pods[i].Create(PODS);
  }

  NodeContainer core[PARALLEL];
  for (int i = 0;i<PARALLEL;i++) {
    core[i].Create(CORE);
  }

  NodeContainer nc_node2pod[PARALLEL][NODES];
  NetDeviceContainer ndc_node2pod[PARALLEL][NODES];

  NodeContainer nc_pod2core[PARALLEL][CORE*PODS];
  NetDeviceContainer ndc_pod2core[PARALLEL][CORE*PODS];
  


  //connect nodes to pods
  for (int i = 0; i < PARALLEL;i++) {
      for (int n = 0; n < NODES; n++) {
          nc_node2pod[i][n] = NodeContainer(pods[i].Get((n/(K/2))/PERPOD), nodes.Get(n));
      }
  }

  //connect pod to core
  for (int i = 0; i < PARALLEL;i++) {
      for (int coreS = 0; coreS < CORE;coreS++) {
          for (int pod = 0; pod < PODS; pod++) {
              nc_pod2core[i][(coreS * CORE) + pod] = NodeContainer(core[i].Get(coreS), pods[i].Get(pod));
          }
      }
  }


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));


  //connect nodes to edges
  for (int i=0; i < PARALLEL; i++) {
      for (int n = 0; n < NODES; n++) {
          ndc_node2pod[i][n] = pointToPoint.Install(nc_node2pod[i][n]);
      }
  }

  //connect pods to core
  for (int i=0; i < PARALLEL; i++) {
    for (int s=0; s < CORE * PODS; s++) {
        ndc_pod2core[i][s] = pointToPoint.Install(nc_pod2core[i][s]);
    }
  }


  InternetStackHelper stack;
  stack.Install (nodes);
  for (int i=0; i<PARALLEL;i++) {
      stack.Install (pods[i]);
      stack.Install (core[i]);
  }

  Ipv4AddressHelper address;
  Ipv4InterfaceContainer node2pods[PARALLEL][NODES];
  Ipv4InterfaceContainer pods2core[PARALLEL][CORE*PODS];

  for (int i=0;i<PARALLEL;i++) {
          std::stringstream addr;
        addr << "10." << 1 + i << ".1.0";
  	address.SetBase(addr.str().c_str(), "255.255.255.255");
      for (int n=0;n<NODES;n++) {
        node2pods[i][n] = address.Assign(ndc_node2pod[i][n]);
      }
      for (int c=0;c<CORE*PODS;c++) {
        pods2core[i][c] = address.Assign(ndc_pod2core[i][c]);
      }
  }




  //Starting Here The question is how do I assign specific IP addresses rather than using the address.Assign counter which automatically assings addresses.
  //
  //
  //bool dred = false;
  //bool raid = false;
  //int dredTrigger = 500; 
  //int raidTrigger = 1000;
  //This is all probably going to die, at some point I will need to build wrappers for both kinds of clients so that I can populate networks will all kinds of applications.
  /*
  if (dred) {
	  serverport = dredTrigger + PARALLEL;
  } else if (raid) {
	  serverport = raidTrigger + PARALLEL;
  }*/
   
  //In D-Redundancy each server listens on an array of ports
  //Further each server listens on an array of addresses... why would that be the case, each IP can use the same port.
  int serverport = 9;
  int clientIndex = 0;
  int serverIndex = 11;
  Address serverIPS[PARALLEL];
  for (int i=0;i<PARALLEL;i++) {
	  serverIPS[i] = node2pods[i][serverIndex].GetAddress(1);
  }
  //DRedundancyServerHelper dServer (serverport, serverIPS,PARALLEL);
  RaidServerHelper dServer (serverport, serverIPS,PARALLEL);

  ApplicationContainer serverApps = dServer.Install (nodes.Get (serverIndex));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));
  
  //map clients to servers 
  //DRedundancyClientHelper dClient (serverport, serverIPS, PARALLEL);
  RaidClientHelper dClient (serverport, serverIPS, PARALLEL);
  dClient.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));
  //dClient.SetAddresses(serverIPS, PARALLEL);

  ApplicationContainer clientApps = dClient.Install (nodes.Get (clientIndex));
  Ptr<RaidClient> drc = DynamicCast<RaidClient>(clientApps.Get(0));
  drc->SetFill("In the days of my youth I was told what it means to be a man-");
  drc->SetAddresses(serverIPS,PARALLEL);
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  //pointToPoint.EnablePcapAll("pods");
  //AsciiTraceHelper ascii;
  //pointToPoint.EnableAsciiAll(ascii.CreateFileStream("pods.tr"));
  

  //Starting Here The question is how do I assign specific IP addresses rather than using the address.Assign counter which automatically assings addresses.
  //
  
  //int measureserverport = 11;
  //int measureClientIndex = 0;
  //int measureServerIndex = 5;
  int coverserverport = 10;
  float clientStart = 2.0;
  float clientStop = 2000.0;

  //SetupRandomMeasureClient(clientStart,clientStop,measureserverport,serverAddress, nodes, measureClientIndex);
  
   //Uniform cover to nearist neighbour
  //SetupUniformCoverTraffic(clientStart, stop, gap, offset, coverserverport, nodes, NODES, 0, node2edge);
  //Uniform cover over edge + agg
  //SetupUniformCoverTraffic(clientStart, stop, gap, offset, coverserverport, nodes, NODES, (K/2), node2edge);
  //Uniform cover over TOR
  //SetupUniformCoverTraffic(clientStart, stop, gap, offset, coverserverport, nodes, NODES, K, node2edge);
  //SetupRandomCoverTraffic(clientStart, stop, gap, offset, coverserverport, nodes, NODES, (K/2) node2edge);
  for (int i=0 ; i < PARALLEL; i++ ){
  	SetupRandomCoverTraffic(
			clientStart, 
			clientStop, 
			CoverNPackets,
			CoverInterval,
		        CoverPacketSize,
			coverserverport + i, 
			nodes, 
			NODES, 
			K, 
			node2pods[i]);
  }
  

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}



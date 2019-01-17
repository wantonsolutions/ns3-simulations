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

#define ECHO 0
#define DRED 1
#define RAID 2

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


//-------------------------------------------------DRedundancy Client--------------------------------------
void InstallDRedClientAttributes(DRedundancyClientHelper *dClient, int maxpackets, double interval, int packetsize) {
  dClient->SetAttribute ("MaxPackets", UintegerValue (maxpackets));
  dClient->SetAttribute ("Interval", TimeValue (Seconds (interval)));
  dClient->SetAttribute ("PacketSize", UintegerValue (packetsize));
}

void InstallRandomDRedClientTransmissions(float start, float stop, int clientIndex, DRedundancyClientHelper *dClient, NodeContainer nodes, Address serverAddress[PARALLEL]) {
  for (float base = start;base < stop; base += 1.0) {
        ApplicationContainer clientApps = dClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
	clientApps.Stop( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
	  Ptr<DRedundancyClient> drc = DynamicCast<DRedundancyClient>(clientApps.Get(0));
	  drc->SetFill("In the days of my youth I was told what it means to be a man-");
	  drc->SetAddresses(serverAddress,PARALLEL);
  }
}

void SetupModularRandomDRedClient(float start, float stop, int serverPort, Address serverAddress[PARALLEL], NodeContainer nodes, int clientIndex, double interval, int packetsize, int maxpackets) {
  DRedundancyClientHelper dClient (serverPort, serverAddress, PARALLEL);
  InstallDRedClientAttributes(&dClient, maxpackets,interval,packetsize);
  InstallRandomDRedClientTransmissions(start,stop,clientIndex,&dClient,nodes,serverAddress);
}

//----------------------------------------------ECHO Client----------------------------------------------------
void InstallEchoClientAttributes(UdpEchoClientHelper *echoClient, int maxpackets, double interval, int packetsize) {
  echoClient->SetAttribute ("MaxPackets", UintegerValue (maxpackets));
  echoClient->SetAttribute ("Interval", TimeValue (Seconds (interval)));
  echoClient->SetAttribute ("PacketSize", UintegerValue (packetsize));
}

void InstallRandomEchoClientTransmissions(float start, float stop, int clientIndex, UdpEchoClientHelper *echoClient, NodeContainer nodes) {
  for (float base = start;base < stop; base += 1.0) {
        ApplicationContainer clientApps = echoClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
	clientApps.Stop( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
  }
}

void InstallUniformEchoClientTransmissions(float start, float stop, float gap, int clientIndex, UdpEchoClientHelper *echoClient, NodeContainer nodes) {
  for (float base = start;base < stop; base += gap) {
  	ApplicationContainer clientApps = echoClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += gap;
	clientApps.Stop( Seconds (base));
  }
}

void SetupModularRandomEchoClient(float start, float stop, int serverPort, Address serverAddress, NodeContainer nodes, int clientIndex, double interval, int packetsize, int maxpackets) {
  //map clients to servers 
  //NS_LOG_INFO("Starting Client Packet Size " << packetsize << " interval " << interval << " nPackets " << maxpackets );
  UdpEchoClientHelper echoClient (serverAddress, serverPort);
  InstallEchoClientAttributes(&echoClient, maxpackets,interval,packetsize);
  InstallRandomEchoClientTransmissions(start,stop,clientIndex,&echoClient,nodes);
}

void SetupRandomCoverTraffic(float clientStart,float clientStop,float serverStart, float serverStop, int NPackets, float interval, int packetsize, int serverport ,NodeContainer nodes, int numNodes, int distance, Ipv4InterfaceContainer **addresses, int mode, Address secondAddrs[NODES][PARALLEL]) {
  for (int i = 0; i < numNodes; i++) {
          int serverindex;
	  bool isClient = (i%2);
	 if (! isClient) {
		serverindex = ((i-1) + (distance)) % numNodes;
	 } else {
		serverindex = i;
	 }

	 /*
          if (isClient){
		  continue;

	  }*/

	  ApplicationContainer serverApps;  


	  if ( ! isClient ) {
		  switch (mode) {
			case ECHO: {
				UdpEchoServerHelper echoServer (serverport);
				serverApps = echoServer.Install (nodes.Get (i));
				break;
		        }
			case DRED: {
				DRedundancyServerHelper dserver (uint16_t(serverport),secondAddrs[serverindex],uint8_t(PARALLEL));
				serverApps = dserver.Install (nodes.Get (i));
				break;
		        }
			case RAID: {
				RaidServerHelper rserver (serverport,secondAddrs[serverindex],PARALLEL);
				serverApps = rserver.Install (nodes.Get (i));
				break;
		        }
		  serverApps.Start (Seconds (serverStart));
		  serverApps.Stop (Seconds (clientStop));
		  }
	  } else {
		  //Pick the server in the nearist pod
		  //Right now the servers only communicate over a single channel serverIPs[0] should be serverIPs[rand()%PARALLEL]
		  switch (mode) {
			case ECHO: {
		  		SetupModularRandomEchoClient(clientStart,clientStop,serverport,secondAddrs[serverindex][0],nodes,i,interval,packetsize,NPackets);
				break;
		        }
			case DRED: {
		  		SetupModularRandomDRedClient(clientStart,clientStop,serverport,secondAddrs[serverindex],nodes,i,interval,packetsize,NPackets);
				break;
		        }
			case RAID: {
				   printf("SETUP MODULAR RANDOM RAID CLIENT NOT YET IMPLEMENTED\n");
				   break;
		        }
		  }
	  }
  }
}


int
main (int argc, char *argv[])
{
  CommandLine cmd;

  //Default vlaues for command line arguments
  uint32_t CoverNPackets = 100;
  float CoverInterval = 0.1;
  uint32_t CoverPacketSize = 128;

  uint32_t ClientProtocolNPackets = 200;
  float ClientProtocolInterval = 0.15;
  uint32_t ClientProtocolPacketSize = 256;

  bool debug = false;

  //Command Line argument debugging code	  
  cmd.AddValue("CoverNPackets", "Number of packets for the cover to echo", CoverNPackets);
  cmd.AddValue("CoverInterval", "Interval at which cover traffic broadcasts", CoverInterval);
  cmd.AddValue("CoverPacketSize", "The Size of the packet used by the cover traffic", CoverPacketSize);

  cmd.AddValue("ClientProtocolNPackets", "Number of packets to echo", ClientProtocolNPackets);
  cmd.AddValue("ClientProtocolInterval", "Interval at which a protocol client makes requests", ClientProtocolInterval);
  cmd.AddValue("ClientProtocolPacketSize", "Interval at which a protocol client makes requests", ClientProtocolPacketSize);

  cmd.AddValue("Debug", "Print all log level info statements for all clients", debug);

  cmd.Parse (argc, argv);

  printf("Client - NPackets %d, baseInterval %f packetSize %d \n",ClientProtocolNPackets,ClientProtocolInterval,ClientProtocolPacketSize);
  printf("Cover - NPackets %d, baseInterval %f packetSize %d \n",CoverNPackets,CoverInterval,CoverPacketSize);
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_WARN);
  LogComponentEnable ("DRedundancyClientApplication", LOG_LEVEL_WARN);
  if (debug) {
	  LogComponentEnable ("DRedundancyClientApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("DRedundancyServerApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("RaidClientApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("RaidServerApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
	  LogComponentEnable ("VarClients", LOG_LEVEL_INFO);
  }

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



  int serverport = 9;
  int clientIndex = 0;
  int serverIndex = 11;


  int coverserverport = 10;
  float serverStart = 1.0;
  float clientStart = 2.0;
  float clientStop = 100.0;

  float duration = clientStop;
  float serverStop = duration;


  Address IPS[NODES][PARALLEL];
  for( int i=0;i<NODES;i++) {
  	for (int j=0;j<PARALLEL;j++) {
	  	IPS[i][j] = node2pods[j][i].GetAddress(1);
	}
  }


  Address serverIPS[PARALLEL];
  for (int i=0;i<PARALLEL;i++) {
	  serverIPS[i] = node2pods[i][serverIndex].GetAddress(1);
  }

  for (int i=0;i<PARALLEL;i++) {
	  //printf("Is this gettting hit \n");
	  NS_LOG_INFO("server addr " << serverIPS[i]);
  }

  //determine which client mode is running

  const int mode = DRED;

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  switch (mode) {
	  case ECHO:
	  {
		  UdpEchoServerHelper dServer (serverport);
		  serverApps = dServer.Install (nodes.Get (serverIndex));

		  //map clients to servers 
		  UdpEchoClientHelper dClient (serverIPS[0], serverport);
		  dClient.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
		  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
		  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));

		  clientApps = dClient.Install (nodes.Get (clientIndex));
		  Ptr<UdpEchoClient> drc = DynamicCast<UdpEchoClient>(clientApps.Get(0));
		  drc->SetFill("In the days of my youth I was told what it means to be a man-");
		  break;
	}
	case DRED:
	  {
		  DRedundancyServerHelper dServer (serverport, serverIPS,PARALLEL);
		  serverApps = dServer.Install (nodes.Get (serverIndex));
		  
		  //map clients to servers 
		  DRedundancyClientHelper dClient (serverport, serverIPS, PARALLEL);
		  dClient.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
		  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
		  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));

		  clientApps = dClient.Install (nodes.Get (clientIndex));
		  Ptr<DRedundancyClient> drc = DynamicCast<DRedundancyClient>(clientApps.Get(0));
		  drc->SetFill("In the days of my youth I was told what it means to be a man-");
		  drc->SetAddresses(serverIPS,PARALLEL);
		  break;
	}
	case RAID:
	  {
		  RaidServerHelper dServer (serverport, serverIPS,PARALLEL);
		  serverApps = dServer.Install (nodes.Get (serverIndex));
		  
		  //map clients to servers 
		  RaidClientHelper dClient (serverport, serverIPS, PARALLEL);
		  dClient.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
		  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
		  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));

		  clientApps = dClient.Install (nodes.Get (clientIndex));
		  Ptr<RaidClient> drc = DynamicCast<RaidClient>(clientApps.Get(0));
		  drc->SetFill("In the days of my youth I was told what it means to be a man-");
		  drc->SetAddresses(serverIPS,PARALLEL);
		  break;
	  }
  }
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  clientApps.Start (Seconds (clientStart));
  clientApps.Stop (Seconds (duration));
  serverApps.Start (Seconds (serverStart));
  serverApps.Stop (Seconds (duration));


  ///!!!!!!!!!!!!!!!!KILLL YOURSELF START HERE IN THE MORNING YOU TIRED BASTARD
  Ipv4InterfaceContainer **node2podsPtr = new Ipv4InterfaceContainer*[PARALLEL];
  for (int i = 0; i< PARALLEL; i++ ) {
	  node2podsPtr[i] = new Ipv4InterfaceContainer[NODES];
	  for (int  j = 0; j <NODES; j++ ) {
		  node2podsPtr[i][j] = node2pods[i][j];
		  NS_LOG_INFO("ADDRESS Print -> (" <<i <<","<<j<<") " << node2podsPtr[i][j].GetAddress(0,0));
	  }
  }

  for (int i = 0; i< PARALLEL; i++ ) {
	  for (int  j = 0; j <NODES; j++ ) {
		  NS_LOG_INFO("ADDRESS COPY -> (" <<i <<","<<j<<") " << node2podsPtr[i][j].GetAddress(0,0));
	  }
  }

	NS_LOG_INFO("Test Print");
         
	SetupRandomCoverTraffic(
			clientStart, 
			clientStop, 
			serverStart,
			serverStop,
			CoverNPackets,
			CoverInterval,
			CoverPacketSize,
			coverserverport,
			nodes, 
			NODES, 
			K, 

			//&node2pods,
			node2podsPtr,
			mode,
			IPS
			);
  

  for (int i = 0; i< PARALLEL; i++ ) {
	  for (int  j = 0; j <NODES; j++ ) {
		  NS_LOG_INFO("ADDRESS COPY -> (" <<i <<","<<j<<") " << node2podsPtr[i][j].GetAddress(0,0));
	  }
  }
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}

//NOTES on how to set up experiments. Each one of the major experiments should
//have some degrees of freedom, and some degrees of configurability. 
//
//Things which should be passed in from the command line include - NPackets -
//NormalPacketSize - Client Rate (constant)
//
// The issue of configurability comes in which determining which nodes are
// running which traffic patterns, and what kinds of workloads they are using.
// Composition should be handeled in script. I cannot think of a useful reason
// to configure this early on, at least untill the efficacy of the protocols
// has been demonstrated.
//
//

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
#include "ns3/csma-module.h"
#include "ns3/applications-module.h"
#include "ns3/multichannel-probe-module.h"
#include "ns3/testmodule-module.h"

#include <string>
#include <fstream>
#include <stdint.h>

#define ECHO 0
#define DRED 1
#define RAID 2

#define CROSS_CORE 0

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


//GLOBALS
  uint32_t CoverNPackets = 100;
  float CoverInterval = 0.1;
  uint32_t CoverPacketSize = 128;

  uint32_t ClientProtocolNPackets = 200;
  float ClientProtocolInterval = 0.15;
  uint32_t ClientProtocolPacketSize = 256;

  double IntervalRatio = .99;

  int mode = ECHO;

  bool debug = false;

  std::string ManifestName = "manifest.config";
  std::string ProbeName = "default.csv";

  const char * CoverNPacketsString = "CoverNPackets";
  const char * CoverIntervalString = "CoverInterval";
  const char * CoverPacketSizeString = "CoverPacketSize";

  const char * ClientProtocolNPacketsString = "ClientProtocolNPackets";
  const char * ClientProtocolIntervalString = "ClientProtocolInterval";
  const char * ClientProtocolPacketSizeString = "ClientProtocolPacketSize";

  const char * IntervalRatioString = "IntervalRatio";

  const char * ManifestNameString = "ManifestName";
  const char * ProbeNameString = "ProbeName";

  const char * DebugString = "Debug";
  const char * ModeString = "Mode";

  const char * KString = "K";
  const char * TopologyString = "Topology";
  const char * Topology = "PFatTree";
  const char * ParallelString = "Parallel";
//\Globals

//-------------------------------------------------DRedundancy Client--------------------------------------
void InstallDRedClientAttributes(DRedundancyClientHelper *dClient, int maxpackets, double interval, int packetsize) {
  dClient->SetAttribute ("MaxPackets", UintegerValue (maxpackets));
  dClient->SetAttribute ("Interval", TimeValue (Seconds (interval)));
  dClient->SetAttribute ("PacketSize", UintegerValue (packetsize));
}

void InstallRandomDRedClientTransmissions(float start, float stop, int clientIndex, DRedundancyClientHelper *dClient, NodeContainer nodes, Address serverAddress[PARALLEL]) {

  ApplicationContainer clientApps = dClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (start));
	clientApps.Stop( Seconds (stop));
	Ptr<DRedundancyClient> drc = DynamicCast<DRedundancyClient>(clientApps.Get(0));
	//drc->SetFill("In the days of my youth I was told what it means to be a man-");
	drc->SetAddresses(serverAddress,PARALLEL);
	drc->SetDistribution(DRedundancyClient::nodist);
	drc->SetIntervalRatio(IntervalRatio);
	//drc->SetDistribution(DRedundancyClient::nodist);
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

void InstallRandomEchoClientTransmissions(float start, float stop, int clientIndex, UdpEchoClientHelper *echoClient, NodeContainer nodes, Address addresses[NODES][PARALLEL], uint16_t Ports[NODES][PARALLEL], int trafficMatrix[NODES][NODES]) {
	/*
  for (float base = start;base < stop; base += 1.0) {
        ApplicationContainer clientApps = echoClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
	clientApps.Stop( Seconds (base));
	base += (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 5;
  }*/
        ApplicationContainer clientApps = echoClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (start));
	clientApps.Stop( Seconds (stop));
	Ptr<UdpEchoClient> ech = DynamicCast<UdpEchoClient>(clientApps.Get(0));
	ech->SetDistribution(UdpEchoClient::nodist);
	ech->SetIntervalRatio(IntervalRatio);

      Ptr<UdpEchoClient> uec = DynamicCast<UdpEchoClient>(clientApps.Get(0));


        //Convert Addresses
        ////TODO Start here, trying to convert one set of pointers to another.
      Address **addrs = new Address*[NODES];
      uint16_t **ports = new uint16_t *[NODES];
      int **tm = new int*[NODES];
      for (int i=0;i<NODES;i++) {
          addrs[i] = &addresses[i][0];
          ports[i] = &Ports[i][0];
          tm[i] = &trafficMatrix[i][0];
      }



      //uec->SetAllAddresses((Address **)(addresses),(uint16_t **)(Ports),PARALLEL,NODES);
      uec->SetAllAddresses(addrs,ports,tm,PARALLEL,NODES);


  
}

void InstallUniformEchoClientTransmissions(float start, float stop, float gap, int clientIndex, UdpEchoClientHelper *echoClient, NodeContainer nodes) {
  for (float base = start;base < stop; base += gap) {
  	ApplicationContainer clientApps = echoClient->Install (nodes.Get (clientIndex));
	clientApps.Start( Seconds (base));
	base += gap;
	clientApps.Stop( Seconds (base));
  }
}

void SetupModularRandomEchoClient(float start, float stop, uint16_t Ports[NODES][PARALLEL], Address addresses[NODES][PARALLEL], int tm[NODES][NODES], NodeContainer nodes, int clientIndex, double interval, int packetsize, int maxpackets) {
  //map clients to servers 
  //NS_LOG_INFO("Starting Client Packet Size " << packetsize << " interval " << interval << " nPackets " << maxpackets );
  UdpEchoClientHelper echoClient (addresses[0][0], int(Ports[0][0]));
  InstallEchoClientAttributes(&echoClient, maxpackets,interval,packetsize);
  InstallRandomEchoClientTransmissions(start,stop,clientIndex,&echoClient,nodes, addresses, Ports, tm);
  
}

void printTM(int tm[NODES][NODES]) {
    printf("\n");
    for (int i =0;i<NODES;i++) {
        for (int j=0;j<NODES;j++) {
            printf("[%2d]",tm[i][j]);
        }
        printf("\n");
    }
}
void zeroTM(int tm[NODES][NODES]) {
    for (int i =0;i<NODES;i++) {
        for (int j=0;j<NODES;j++) {
            tm[i][j]=0;
        }
    }
}

void populateTrafficMatrix(int tm[NODES][NODES], int pattern) {
    zeroTM(tm);
    switch (pattern) {
        case CROSS_CORE:

            //This relies on the fact taht clients are even numbers and servers
            //are odd, ie every client should have a relative server beside
            //them accounting for the (i-1) as the first term of the server
            //index equasion. THe second term adds the index to halfway across
            //the fat tree, the last term mods the server index by the size of the fat-tree.
            
            for (int i=0;i<NODES;i++) {
                int serverindex = ((i-1) + ((K*K*K)/8)) % ((K*K*K)/4); //TODO Debug this might not be right for all fat trees
                tm[i][serverindex] = 1;
            }
    }
    printTM(tm);
}


void SetupRandomCoverTraffic(float clientStart,float clientStop,float serverStart, float serverStop, int NPackets, float interval, int packetsize, int serverport ,NodeContainer nodes, int numNodes, int tm[NODES][NODES], Ipv4InterfaceContainer **addresses, int mode, Address secondAddrs[NODES][PARALLEL], uint16_t Ports [NODES][PARALLEL]) {

  bool singlePair = false;
  for (int i = 0; i < numNodes; i++) {
          int serverindex;

	      bool isClient = (i%2);
	   
         /*  Killed by the traffic matrix
          */
		 if (! isClient) {
			serverindex = i;
		 } else {
			serverindex = ((i-1) + ((K*K*K)/8)) % numNodes;
		 }
         // Kept alive only for the sake of DRedClient -- 

	  if ( ! isClient ) {
	  	  ApplicationContainer serverApps;  
		  switch (mode) {
			case ECHO: {
				UdpEchoServerHelper echoServer (serverport);
				serverApps = echoServer.Install (nodes.Get (i));
				break;
		        }
			case DRED: {
				DRedundancyServerHelper dserver (uint16_t(serverport),secondAddrs[i],uint8_t(PARALLEL));
				serverApps = dserver.Install (nodes.Get (i));
				break;
		        }
			case RAID: {
				RaidServerHelper rserver (serverport,secondAddrs[i],PARALLEL);
				serverApps = rserver.Install (nodes.Get (i));
				break;
		        }
		  serverApps.Start (Seconds (serverStart));
		  serverApps.Stop (Seconds (clientStop));
		  }
	  } else {
          if (i>1 && singlePair) {
              continue;
          }
		  //Pick the server in the nearist pod
		  //Right now the servers only communicate over a single channel serverIPs[0] should be serverIPs[rand()%PARALLEL]
		  switch (mode) {
			case ECHO: {
			        printf("setting up echo");
                                                                         //secondAddrs[distance][rand()%PARALLEL]
		  		SetupModularRandomEchoClient(clientStart,clientStop,Ports,secondAddrs,tm,nodes,i,interval,packetsize,NPackets);
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

void translateIp(int base, int* a, int* b, int* c, int* d) {
	*a = base % 256;
	base = base / 256;
	*b = base % 256;
	base = base / 256;
	*c = base % 256;
	base = base / 256;
	*d = base % 256;
	return;
}



int
main (int argc, char *argv[])
{
  CommandLine cmd;

  //Default vlaues for command line arguments

  Config::SetDefault ("ns3::Ipv4GlobalRouting::RandomEcmpRouting",BooleanValue(true));
  Config::SetDefault ("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue (true));

  //Command Line argument debugging code	  
  cmd.AddValue(CoverNPacketsString, "Number of packets for the cover to echo", CoverNPackets);
  cmd.AddValue(CoverIntervalString, "Interval at which cover traffic broadcasts", CoverInterval);
  cmd.AddValue(CoverPacketSizeString, "The Size of the packet used by the cover traffic", CoverPacketSize);

  cmd.AddValue(ClientProtocolNPacketsString, "Number of packets to echo", ClientProtocolNPackets);
  cmd.AddValue(ClientProtocolIntervalString, "Interval at which a protocol client makes requests", ClientProtocolInterval);
  cmd.AddValue(ClientProtocolPacketSizeString, "Interval at which a protocol client makes requests", ClientProtocolPacketSize);

  cmd.AddValue(IntervalRatioString, "Ratio at which the ratio of client requests increases", IntervalRatio);

  cmd.AddValue(ManifestNameString, "Then name of the ouput manifest (includes all configurations)", ManifestName);
  cmd.AddValue(ProbeNameString, "Then name of the output probe CSV", ProbeName);

  cmd.AddValue(DebugString, "Print all log level info statements for all clients", debug);
  cmd.AddValue(ModeString, "The Composition of the clients ECHO=0 DRED=1 RAID=2", mode);

  printf("Parsing");
  cmd.Parse (argc, argv);
  printf("Done Pares");
  //mode = DRED;
  //
  //Open a file to write out manifest
  std::string manifestFilename = ManifestName;
  std::ios_base::openmode openmode = std::ios_base::out | std::ios_base::trunc;
  //ofstream->open (manifestFilename.c_str (), openmode);
  OutputStreamWrapper StreamWrapper = OutputStreamWrapper(manifestFilename, openmode);
  //StreamWrapper->SetStream(ofstream);
	std::ostream *stream = StreamWrapper.GetStream();

	*stream << CoverNPacketsString << ":" << CoverNPackets << "\n";
	*stream << CoverIntervalString << ":" << CoverInterval <<"\n";
	*stream << CoverPacketSizeString << ":" << CoverPacketSize <<"\n";
	*stream << ClientProtocolNPacketsString<< ":" << ClientProtocolNPackets <<"\n";
	*stream << ClientProtocolIntervalString << ":" << ClientProtocolInterval <<"\n";
	*stream << ClientProtocolPacketSizeString << ":" << ClientProtocolPacketSize <<"\n";
	*stream << IntervalRatioString << ":" << IntervalRatio <<"\n";
	*stream << ManifestNameString << ":" << ManifestName <<"\n";
	*stream << DebugString << ":" << debug <<"\n";
	*stream << ModeString << ":" << mode <<"\n";
	*stream << KString << ":" << K <<"\n";
	*stream << TopologyString << ":" << Topology <<"\n";
	*stream << ParallelString << ":" << PARALLEL <<"\n";



  //printf("Client - NPackets %d, baseInterval %f packetSize %d \n",ClientProtocolNPackets,ClientProtocolInterval,ClientProtocolPacketSize);
  //printf("Cover - NPackets %d, baseInterval %f packetSize %d \n",CoverNPackets,CoverInterval,CoverPacketSize);
  
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
              nc_pod2core[i][(coreS*PODS) + pod] = NodeContainer(core[i].Get(coreS), pods[i].Get(pod));
          }
 	}
  }


  int BaseRate = 1;
  int ModRate = BaseRate / PARALLEL;	  
  std::stringstream datarate;
  datarate << ModRate << "Mbps";
  //printf("Data Rate %s\n", datarate.str().c_str());	 
 

  //Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue ("100p"));
  //Config::SetDefault ("ns3::QueueBase::MaxSize", QueueSizeValue(QueueSize("1p")));
  //
  InternetStackHelper stack;
  stack.Install (nodes);
  for (int i=0; i<PARALLEL;i++) {
      stack.Install (pods[i]);
      stack.Install (core[i]);
  }

  PointToPointHelper pointToPoint;
  PointToPointHelper pointToPoint2;
  TrafficControlHelper tch;
  int linkrate = 1;
  int queuedepth = 1;

  pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue(std::to_string(linkrate) + "Gbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("1.0us"));
  ////
  pointToPoint2.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint2.SetDeviceAttribute ("DataRate", StringValue(std::to_string(linkrate) + "Gbps"));
  pointToPoint2.SetChannelAttribute ("Delay", StringValue ("1.0us"));

  uint16_t handle = tch.SetRootQueueDisc("ns3::FifoQueueDisc");
  tch.AddInternalQueues(handle, 1, "ns3::DropTailQueue", "MaxSize", StringValue (std::to_string(queuedepth) + "p"));



//pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
  //connect nodes to edges
  for (int i=0; i < PARALLEL; i++) {
      for (int n = 0; n < NODES; n++) {
          ndc_node2pod[i][n] = pointToPoint.Install(nc_node2pod[i][n]);
	  //Ptr<DropTailQueue> queue = DynamicCast<DropTailQueue> (DynamicCast<PointToPointNetDevice> (ndc_node2pod[i][n].Get (0))->GetQueue ());
          //queue->SetAttribute ("MaxPackets", UintegerVbaps alue (250));
	  
      }
  }


//pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue("1p"));
  //connect pods to core
  for (int i=0; i < PARALLEL; i++) {
    for (int s=0; s < CORE * PODS; s++) {
        ndc_pod2core[i][s] = pointToPoint2.Install(nc_pod2core[i][s]);
    }
  }




  //Assign queues AFTER the stack install (not sure why)

  //tch.Install(ndc_node2pod[0][0].Get(0));
  //tch.Install(ndc_node2pod[0][0].Get(1));
  for (int i=0; i < PARALLEL; i++) {
      for (int n = 0; n < NODES; n++) {
          tch.Install(ndc_node2pod[i][n].Get(0));
          tch.Install(ndc_node2pod[i][n].Get(1));
	  
      }
  }

  for (int i=0; i < PARALLEL; i++) {
    for (int s=0; s < CORE * PODS; s++) {
        tch.Install(ndc_pod2core[i][s].Get(0));
        tch.Install(ndc_pod2core[i][s].Get(1));
    }
  }
  


  Ipv4AddressHelper address;
  Ipv4InterfaceContainer node2pods[PARALLEL][NODES];
  Ipv4InterfaceContainer pods2core[PARALLEL][CORE*PODS];

  int a = 1;
  int b = 0;
  int c = 0;
  int d = 10;
  int total = 1;
  for (int i=0;i<PARALLEL;i++) {
        //n2pAddr << "10." << 1 + i << ".1.0";
	      for (int n=0;n<NODES;n++) {
            std::stringstream n2pAddr;
            translateIp(total,&a,&b,&c,&d);
            n2pAddr << c << "." << i << "." << b << "." << a;
            //printf("%s\n",n2pAddr.str().c_str());
            address.SetBase(n2pAddr.str().c_str(), "255.255.255.255");
	      	node2pods[i][n] = address.Assign(ndc_node2pod[i][n]);
            total += K;
	      }
        //p2cAddr << "10." << 1 + i << ".128.0";
	      for (int r=0;r<CORE*PODS;r++) {
            std::stringstream p2cAddr;
            translateIp(total,&a,&b,&c,&d);
            p2cAddr << c << "." << i << "." << b << "." << a;
            //printf("%s(%d/%d)\n",p2cAddr.str().c_str(),r,CORE*PODS);
            //address.SetBase(p2cAddr.str().c_str(), "255.255.255.255");
            address.SetBase(p2cAddr.str().c_str(), "255.255.255.255");
            pods2core[i][r] = address.Assign(ndc_pod2core[i][r]);
            total += K;
	      }
  }
////////////////////////////////////////////////////////////////////////////////////
//Setup Clients
///////////////////////////////////////////////////////////////////////////////////
  //int serverport = 9;
  //int clientIndex = 0;
  //int serverIndex = 11;


  int coverserverport = 10;
  float serverStart = 0.0;
  float clientStart = 0.0;

  float clientStop = 0.001;

  float duration = clientStop;


  Address IPS[NODES][PARALLEL];
  uint16_t Ports[NODES][PARALLEL];
  for( int i=0;i<NODES;i++) {
  	for (int j=0;j<PARALLEL;j++) {
	  	IPS[i][j] = node2pods[j][i].GetAddress(1);
        Ports[i][j] = uint16_t(coverserverport);
	}
  }

  int trafficMatrix[NODES][NODES];
  int pattern = CROSS_CORE;
  populateTrafficMatrix(trafficMatrix, pattern);

/*
  Address serverIPS[PARALLEL];
  for (int i=0;i<PARALLEL;i++) {
	  serverIPS[i] = node2pods[i][serverIndex].GetAddress(1);
  }

  for (int i=0;i<PARALLEL;i++) {
	  //printf("Is this gettting hit \n");
	  NS_LOG_INFO("server addr " << serverIPS[i]);
  }
*/
  //determine which client mode is running
  //pointToPoint.EnablePcapAll (ProbeName);

  Ptr<MultichannelProbe> mcp = CreateObject<MultichannelProbe> (ProbeName);
  mcp->SetAttribute ("Interval", StringValue("1s"));
  mcp->AttachAll ();
  mcp->Stop(Seconds(duration));

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  //HACK REMOVE
  int PacketSize = 1472;
  float Rate = 1.0 / ((PARALLEL * (float(linkrate) * (1000000000.0))) / (float(PacketSize) * 8.0));
  float MaxInterval = Rate * 1.0;
  printf("Rate %f\n",Rate);
  ClientProtocolInterval = MaxInterval;
  CoverInterval = MaxInterval;

  /*
  switch (mode) {
	  case ECHO:
	  {
		  UdpEchoServerHelper dServer (serverport);
		  serverApps = dServer.Install (nodes.Get (serverIndex));

		  //map clients to servers 
		  UdpEchoClientHelper dClient (serverIPS[rand()%PARALLEL], serverport);
		  dClient.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
		  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
		  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));

		  clientApps = dClient.Install (nodes.Get (clientIndex));
		  Ptr<UdpEchoClient> drc = DynamicCast<UdpEchoClient>(clientApps.Get(0));
		  //drc->SetFill("In the days of my youth I was told what it means to be a man-");
		  break;
	}ce/ns-allinone-3.29/ns-3.29$ ./waf
Waf: Entering directory
	case DRED:
	  {
		  NS_LOG_INFO("Running in " << mode << " mode ");
		  DRedundancyServerHelper dServer (serverport, serverIPS,PARALLEL);
		  serverApps = dServer.Install (nodes.Get (serverIndex));
		  
		  //map clients to servers 
		  DRedundancyClientHelper dClient (serverport, serverIPS, PARALLEL);
		  dClient.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
		  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
		  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));

		  clientApps = dClient.Install (nodes.Get (clientIndex));
		  Ptr<DRedundancyClient> drc = DynamicCast<DRedundancyClient>(clientApps.Get(0));
		  //drc->SetFill("In the days of my youth I was told what it means to be a man-");
		  drc->SetDistribution(DRedundancyClient::incremental);
		  drc->SetAddresses(serverIPS,PARALLEL);
		  drc->SetIntervalRatio(IntervalRatio);
		  break;
	}
	case RAID:
	  {
		  RaidServerHelper dServer (serverport, serverIPS,PARALLEL);
		  serverApps = dServer.Install (nodes.Get (serverIndex));
		  
		  //map clients to servers 
		  RaidClientHelper dClient (serverport, serverIPS, PARALLEL);
		  dClien32768t.SetAttribute ("MaxPackets", UintegerValue (ClientProtocolNPackets));
		  dClient.SetAttribute ("Interval", TimeValue (Seconds (ClientProtocolInterval)));
		  dClient.SetAttribute ("PacketSize", UintegerValue (ClientProtocolPacketSize));

		  clientApps = dClient.Install (nodes.Get (clientIndex));
		  Ptr<RaidClient> drc = DynamicCast<RaidClient>(clientApps.Get(0));
		  //drc->SetFill("In the days of my youth I was told what it means to be a man-");
		  drc->SetAddresses(serverIPS,PARALLEL);
		  break;
	  }
  }*/
  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  clientApps.Start (Seconds (clientStart));
  clientApps.Stop (Seconds (duration));
  serverApps.Start (Seconds (serverStart));
  serverApps.Stop (Seconds (duration));


  Ipv4InterfaceContainer **node2podsPtr = new Ipv4InterfaceContainer*[PARALLEL];
  for (int i = 0; i< PARALLEL; i++ ) {
	  node2podsPtr[i] = new Ipv4InterfaceContainer[NODES];
	  for (int  j = 0; j <NODES; j++ ) {
		  node2podsPtr[i][j] = node2pods[i][j];
	  }
  }



         
	SetupRandomCoverTraffic(
			clientStart, 
			duration, 
			serverStart,
			duration,
			CoverNPackets,
			CoverInterval,
			CoverPacketSize,
			coverserverport,
			nodes, 
			NODES, //total nodes
			trafficMatrix, //distance

			//&node2pods,
			node2podsPtr,
			mode,
			IPS,
            Ports
			);
  


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

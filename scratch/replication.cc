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

#define TCP 0

#define CROSS_CORE 0

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("VarClients");

const int K = 4;
const int PODS = K;
const int EDGE = (K/2);
const int EDGES = PODS * EDGE;
const int AGG = (K/2);
const int AGGS = PODS * AGG;
const int CORE = (K/2)*(K/2);
const int NODE = K/2 ;
const int NODES = PODS * EDGE * NODE ;

//GLOBALS
uint32_t CoverNPackets = 100;
float CoverInterval = 0.1;
uint32_t CoverPacketSize = 128;

uint32_t ClientProtocolNPackets = 200;
float ClientProtocolInterval = 0.15;
uint32_t ClientProtocolPacketSize = 256;

double IntervalRatio = .99;

int mode = TCP;

bool debug = false;

std::string ManifestName = "manifest.config";
std::string ProbeName = "default.csv";

const char *CoverNPacketsString = "CoverNPackets";
const char *CoverIntervalString = "CoverInterval";
const char *CoverPacketSizeString = "CoverPacketSize";

const char *ClientProtocolNPacketsString = "ClientProtocolNPackets";
const char *ClientProtocolIntervalString = "ClientProtocolInterval";
const char *ClientProtocolPacketSizeString = "ClientProtocolPacketSize";

const char *IntervalRatioString = "IntervalRatio";

const char *ManifestNameString = "ManifestName";
const char *ProbeNameString = "ProbeName";

const char *DebugString = "Debug";
const char *ModeString = "Mode";

const char *KString = "K";
const char *TopologyString = "Topology";
const char *Topology = "PFatTree";
const char *ParallelString = "Parallel";
//\Globals


//----------------------------------------------ECHO Client----------------------------------------------------
void InstallEchoClientAttributes(UdpEchoClientHelper *echoClient, int maxpackets, double interval, int packetsize)
{
  echoClient->SetAttribute("MaxPackets", UintegerValue(maxpackets));
  echoClient->SetAttribute("Interval", TimeValue(Seconds(interval)));
  echoClient->SetAttribute("PacketSize", UintegerValue(packetsize));
}

void InstallRandomEchoClientTransmissions(float start, float stop, int clientIndex, UdpEchoClientHelper *echoClient, NodeContainer nodes, Address addresses[NODES], uint16_t Ports[NODES], int trafficMatrix[NODES][NODES])
{
  
  ApplicationContainer clientApps = echoClient->Install(nodes.Get(clientIndex));
  clientApps.Start(Seconds(start));
  clientApps.Stop(Seconds(stop));
  Ptr<UdpEchoClient> ech = DynamicCast<UdpEchoClient>(clientApps.Get(0));
  ech->SetDistribution(UdpEchoClient::nodist);
  ech->SetIntervalRatio(IntervalRatio);
  ech->SetParallel(false);

  Ptr<UdpEchoClient> uec = DynamicCast<UdpEchoClient>(clientApps.Get(0));

  //Convert Addresses
  ////TODO Start here, trying to convert one set of pointers to another.
  Address *addrs = new Address [NODES];
  uint16_t *ports = new uint16_t [NODES];
  int **tm = new int *[NODES];
  for (int i = 0; i < NODES; i++)
  {
    addrs[i] = addresses[i];
    ports[i] = Ports[i];
    tm[i] = &trafficMatrix[i][0];
  }
  //uec->SetAllAddresses((Address **)(addresses),(uint16_t **)(Ports),PARALLEL,NODES);
  //uec->SetAllAddresses(addrs, ports, tm, PARALLEL, NODES);
  uec->SetAllAddresses(addrs, ports, tm, NODES);
}

void InstallUniformEchoClientTransmissions(float start, float stop, float gap, int clientIndex, UdpEchoClientHelper *echoClient, NodeContainer nodes)
{
  for (float base = start; base < stop; base += gap)
  {
    ApplicationContainer clientApps = echoClient->Install(nodes.Get(clientIndex));
    clientApps.Start(Seconds(base));
    base += gap;
    clientApps.Stop(Seconds(base));
  }
}

void SetupModularRandomEchoClient(float start, float stop, uint16_t Ports[NODES], Address addresses[NODES], int tm[NODES][NODES], NodeContainer nodes, int clientIndex, double interval, int packetsize, int maxpackets)
{
  //map clients to servers
  //NS_LOG_INFO("Starting Client Packet Size " << packetsize << " interval " << interval << " nPackets " << maxpackets );
  UdpEchoClientHelper echoClient(addresses[0], int(Ports[0]));
  InstallEchoClientAttributes(&echoClient, maxpackets, interval, packetsize);
  InstallRandomEchoClientTransmissions(start, stop, clientIndex, &echoClient, nodes, addresses, Ports, tm);
}

void printTM(int tm[NODES][NODES])
{
  printf("\n");
  for (int i = 0; i < NODES; i++)
  {
    for (int j = 0; j < NODES; j++)
    {
      printf("[%2d]", tm[i][j]);
    }
    printf("\n");
  }
}

void zeroTM(int tm[NODES][NODES])
{
  for (int i = 0; i < NODES; i++)
  {
    for (int j = 0; j < NODES; j++)
    {
      tm[i][j] = 0;
    }
  }
}

void populateTrafficMatrix(int tm[NODES][NODES], int pattern)
{
  zeroTM(tm);
  switch (pattern)
  {
  case CROSS_CORE:

    //This relies on the fact that clients are even numbers and servers
    //are odd, ie every client should have a relative server beside
    //them accounting for the (i-1) as the first term of the server
    //index equasion. THe second term adds the index to halfway across
    //the fat tree, the last term mods the server index by the size of the fat-tree.

    for (int i = 0; i < NODES; i++)
    {
      int serverindex = ((i - 1) + ((K * K * K) / 8)) % ((K * K * K) / 4); //TODO Debug this might not be right for all fat trees
      tm[i][serverindex] = 1;
    }
  }
  printTM(tm);
}

void SetupTraffic(float clientStart, float clientStop, float serverStart, float serverStop, int NPackets, float interval, int packetsize, int serverport, NodeContainer nodes, int numNodes, int tm[NODES][NODES], Ipv4InterfaceContainer *addresses, int mode, Address secondAddrs[NODES], uint16_t Ports[NODES]) {
  //For reference to this function check out SetupRandomCoverTraffic in pfattree.cc
  int clientIndex = 0;
  int serverIndex = 1;
  //Setup Server

  //in this setup we are ignoring trafic matricies completely and just performing a send between a single client and a single server

  ApplicationContainer serverApps;
  UdpEchoServerHelper echoServer(serverport);
  serverApps = echoServer.Install(nodes.Get(serverIndex));

  //Setup client 
  SetupModularRandomEchoClient(clientStart, clientStop, Ports, secondAddrs, tm, nodes, clientIndex, interval, packetsize, NPackets);
}


void translateIp(int base, int *a, int *b, int *c, int *d)
{
  *a = base % 256;
  base = base / 256;
  *b = base % 256;
  base = base / 256;
  *c = base % 256;
  base = base / 256;
  *d = base % 256;
  return;
}

int main(int argc, char *argv[])
{
  CommandLine cmd;

  //Default vlaues for command line arguments

  Config::SetDefault("ns3::Ipv4GlobalRouting::RandomEcmpRouting", BooleanValue(true));
  Config::SetDefault("ns3::Ipv4GlobalRouting::RespondToInterfaceEvents", BooleanValue(true));

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
  cmd.Parse(argc, argv);
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
  *stream << CoverIntervalString << ":" << CoverInterval << "\n";
  *stream << CoverPacketSizeString << ":" << CoverPacketSize << "\n";
  *stream << ClientProtocolNPacketsString << ":" << ClientProtocolNPackets << "\n";
  *stream << ClientProtocolIntervalString << ":" << ClientProtocolInterval << "\n";
  *stream << ClientProtocolPacketSizeString << ":" << ClientProtocolPacketSize << "\n";
  *stream << IntervalRatioString << ":" << IntervalRatio << "\n";
  *stream << ManifestNameString << ":" << ManifestName << "\n";
  *stream << DebugString << ":" << debug << "\n";
  *stream << ModeString << ":" << mode << "\n";
  *stream << KString << ":" << K << "\n";
  *stream << TopologyString << ":" << Topology << "\n";

  //printf("Client - NPackets %d, baseInterval %f packetSize %d \n",ClientProtocolNPackets,ClientProtocolInterval,ClientProtocolPacketSize);
  //printf("Cover - NPackets %d, baseInterval %f packetSize %d \n",CoverNPackets,CoverInterval,CoverPacketSize);

  Time::SetResolution(Time::NS);
  LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_WARN);
  LogComponentEnable("DRedundancyClientApplication", LOG_LEVEL_WARN);
  if (debug)
  {
    LogComponentEnable("DRedundancyClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("DRedundancyServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RaidClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("RaidServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    LogComponentEnable("VarClients", LOG_LEVEL_INFO);
  }


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


  int BaseRate = 1;
  int ModRate = BaseRate;
  std::stringstream datarate;
  datarate << ModRate << "Mbps";
  //printf("Data Rate %s\n", datarate.str().c_str());

  //Config::SetDefault ("ns3::QueueBase::MaxSize", StringValue ("100p"));
  //Config::SetDefault ("ns3::QueueBase::MaxSize", QueueSizeValue(QueueSize("1p")));
  //

  
  InternetStackHelper stack;
  stack.Install (nodes);
  stack.Install (edge);
  stack.Install (agg);
  stack.Install (core);

  PointToPointHelper pointToPoint;
  PointToPointHelper pointToPoint2;

  TrafficControlHelper tch;
  int linkrate = 1;
  int queuedepth = 1;

  pointToPoint.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint.SetDeviceAttribute("DataRate", StringValue(std::to_string(linkrate) + "Gbps"));
  pointToPoint.SetChannelAttribute("Delay", StringValue("1.0us"));
  ////
  pointToPoint2.SetQueue("ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));
  pointToPoint2.SetDeviceAttribute("DataRate", StringValue(std::to_string(linkrate) + "Gbps"));
  pointToPoint2.SetChannelAttribute("Delay", StringValue("1.0us"));

  uint16_t handle = tch.SetRootQueueDisc("ns3::FifoQueueDisc");
  tch.AddInternalQueues(handle, 1, "ns3::DropTailQueue", "MaxSize", StringValue(std::to_string(queuedepth) + "p"));


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
      printf("%d\n",e);
      ndc_edge2agg[e] = pointToPoint.Install(nc_edge2agg[e]);
  }
  //connect agg to cores
  for (int s = 0;s < CORE * PODS;s++) {
      ndc_agg2core[s] = pointToPoint.Install(nc_agg2core[s]);
  }

  //Assign queues AFTER the stack install (not sure why)

  //connect nodes to edges
  for (int n = 0; n < NODES; n++) {
      tch.Install(ndc_node2edge[n].Get(0));
      tch.Install(ndc_node2edge[n].Get(1));
  }


  //connect edges to agg
  for (int pod = 0; pod < PODS; pod++) {
      for (int edgeS = 0; edgeS < EDGE; edgeS++) {
          for (int aggS = 0; aggS < AGG; aggS++) {
              //int aggIndex = pod*AGG + aggS;
              //int edgeIndex = pod*EDGE + edgeS;
              int link = (pod * (K/2)*(K/2)) + (edgeS * EDGE) + aggS;
              //int link = (pod * PODS) + (aggIndex) + edgeIndex;
              //printf("link %d, aggIndex %d, edgeIndex %d\n",link,aggIndex,edgeIndex);
              tch.Install(ndc_edge2agg[link].Get(0));
              tch.Install(ndc_edge2agg[link].Get(1));
          }
      }
  }

  //connect agg to core
  for (int coreS = 0; coreS < CORE;coreS++) {
      for (int pod = 0; pod < PODS; pod++) {
              tch.Install(ndc_agg2core[(coreS * CORE) + pod].Get(0));
              tch.Install(ndc_agg2core[(coreS * CORE) + pod].Get(1));
      }
  }


  Ipv4AddressHelper address;
  Ipv4InterfaceContainer node2edge[NODES];
  Ipv4InterfaceContainer edge2agg[EDGE*AGG*PODS];
  Ipv4InterfaceContainer agg2core[CORE*PODS];

  //TODO Assign address as described in the fat tree paper, code for doing so is prototyped in pfattree.c
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

  Address IPS[NODES];
  uint16_t Ports[NODES];
  for (int i = 0; i < NODES; i++)
  {
      IPS[i] = node2edge[i].GetAddress(1);
      Ports[i] = uint16_t(coverserverport);
  }

  int trafficMatrix[NODES][NODES];
  int pattern = CROSS_CORE;
  populateTrafficMatrix(trafficMatrix, pattern);


  Ptr<MultichannelProbe> mcp = CreateObject<MultichannelProbe>(ProbeName);
  mcp->SetAttribute("Interval", StringValue("1s"));
  mcp->AttachAll();
  mcp->Stop(Seconds(duration));

  ApplicationContainer clientApps;
  ApplicationContainer serverApps;

  //HACK REMOVE
  int PacketSize = 1472;
  //float Rate = 1.0 / ((PARALLEL * (float(linkrate) * (1000000000.0))) / (float(PacketSize) * 8.0));
  float Rate = 1.0 / (((float(linkrate) * (1000000000.0))) / (float(PacketSize) * 8.0));
  float MaxInterval = Rate * 1.0;
  printf("Rate %f\n", Rate);
  ClientProtocolInterval = MaxInterval;
  CoverInterval = MaxInterval;

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  clientApps.Start(Seconds(clientStart));
  clientApps.Stop(Seconds(duration));
  serverApps.Start(Seconds(serverStart));
  serverApps.Stop(Seconds(duration));

    Ipv4InterfaceContainer *node2edgePtr = new Ipv4InterfaceContainer[NODES];
    for (int i = 0; i < NODES; i++)
    {
      node2edgePtr[i] = node2edge[i];
    }


  SetupTraffic(
      clientStart,
      duration,
      serverStart,
      duration,
      CoverNPackets,
      CoverInterval,
      CoverPacketSize,
      coverserverport,
      nodes,
      NODES,         //total nodes
      trafficMatrix, //distance

      //&node2pods,
      node2edgePtr,
      mode,
      IPS,
      Ports);

  Simulator::Run();
  Simulator::Destroy();
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

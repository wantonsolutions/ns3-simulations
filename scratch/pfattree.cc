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





int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  
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
  dClient.SetAttribute ("MaxPackets", UintegerValue (5000));
  dClient.SetAttribute ("Interval", TimeValue (Seconds (2)));
  dClient.SetAttribute ("PacketSize", UintegerValue (1024));
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

  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}



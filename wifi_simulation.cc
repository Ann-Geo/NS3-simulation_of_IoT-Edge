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
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-classifier.h"
#include "ns3/flow-probe.h"
#include "ns3/netanim-module.h"

// Default Network Topology
//
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1   n2   n3   n4
//                   point-to-point  |    |    |    |
//                                   ================
//                                     LAN 10.1.2.0






// Simulated Network Topology
//
// Number of wifi or csma nodes can be increased up to 250
//                          |
//                 Rank 0   |   Rank 1
// -------------------------|----------------------------
//   Wifi 10.1.3.0
//                 AP
//  *    *    *    *
//  |    |    |    |    10.1.1.0
// n5   n6   n7   n0 -------------- n1
//                   point-to-point  |    
//                                   =
//                                   









//ns-3 project is implemented in a C++ namespace called ns3
using namespace ns3;



//declares a logging component called ThirdScriptExample that allows you to enable and disable console
//message logging by reference to the name.
NS_LOG_COMPONENT_DEFINE ("ThirdScriptExample");


//main function starts
int 
main (int argc, char *argv[])
{

//default values for command line arguments
  bool verbose = true;
  uint32_t nCsma = 0;
  uint32_t nWifi = 1;
  bool tracing = true;


//can pass these arguments through terminal
  CommandLine cmd;
  cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices", nCsma);
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

//parse the command line arguments
  cmd.Parse (argc,argv);

  // Check for valid number of csma or wifi nodes
  // 250 should be enough
  if (nWifi > 250 || nCsma > 250)
    {
      std::cout << "Too many wifi or csma nodes, no more than 250 each." << std::endl;
      return 1;
    }

  if (verbose)
    {

//enable two logging components that are built into the 
//Echo Client and Echo Server applications
//enable debug logging at the INFO level for echo clients and servers. 
//This will result in the application printing out messages as packets are sent
//and received during the simulation.
      LogComponentEnable ("BulkSendApplication", LOG_LEVEL_INFO);
    }


//create the ns-3 Node objects that will represent the computers in
//the simulation. Nodes are the key abstractions
/*
Nodes represents a computer to which we are going to
add things like protocol stacks, applications and peripheral cards. The NodeContainer topology helper provides a
convenient way to create, manage and access any Node objects that we create in order to run a simulation. The first
line just declares a NodeContainer which we call nodes. The second line calls the Create method on the
nodes object and asks the container to create two nodes.
*/
//Here two point to point nodes are created
  NodeContainer p2pNodes;
  p2pNodes.Create (2);


//instantiates a PointToPointHelper object on the stack
/*
two of our key abstractions
are the NetDevice and the Channel. In the real world, these terms correspond 
roughly to peripheral cards and network cables.
*/
  PointToPointHelper pointToPoint;

//tells the PointToPointHelper object to use the value “5Mbps” (five megabits per second) as the “DataRate”
//when it creates a PointToPointNetDevice object.
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Gbps"));

//tells the PointToPointHelper to use the value “2ms” (two milliseconds) as the value of the propagation delay of
//every point to point channel it subsequently creates.
  //pointToPoint.SetChannelAttribute ("Delay", StringValue ("2us"));
pointToPoint.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (10)));

//We will need to have a list of all of the NetDevice objects that are created, so we use a NetDeviceContainer to
//hold them just as we used a NodeContainer to hold the nodes we created
  NetDeviceContainer p2pDevices;
  p2pDevices = pointToPoint.Install (p2pNodes);



//Next, we declare another NodeContainer to hold the nodes that will be part of the bus (CSMA) network. First, we
//just instantiate the container object itself.
/*
The next line of code Gets the first node (as in having an index of one) from the point-to-point node container and
adds it to the container of nodes that will get CSMA devices. The node in question is going to end up with a point-
to-point device and a CSMA device. We then create a number of “extra” nodes that compose the remainder of the
CSMA network. Since we already have one node in the CSMA network – the one that will have both a point-to-point
and CSMA net device, the number of “extra” nodes means the number nodes you desire in the CSMA section minus
one.
*/
  NodeContainer csmaNodes;
  csmaNodes.Add (p2pNodes.Get (1));
  csmaNodes.Create (nCsma);

//We first set the data rate to 100 megabits per second,
//and then set the speed-of-light delay of the channel to 6560 nano-seconds
  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", StringValue ("10Gbps"));
  csma.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (10)));


/*
Just as we created a NetDeviceContainer to hold the devices created by the PointToPointHelper we create
a NetDeviceContainer to hold the devices created by our CsmaHelper.
*/
  NetDeviceContainer csmaDevices;
  csmaDevices = csma.Install (csmaNodes);


/*
Next, we are going to create the nodes that will be part of the Wi-Fi network. We are going to create a number
of “station” nodes as specified by the command line argument, and we are going to use the “leftmost” node of the
point-to-point link as the node for the access point.
*/

  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = p2pNodes.Get (0);


/*
constructs the wifi devices and the interconnection channel between these wifi nodes. First, we
configure the PHY and channel helpers:
*/

YansWifiChannelHelper wifiChannelHelper = YansWifiChannelHelper::Default ();
//Ptr<WifiChannel> wifiChannel = wifiChannelHelper.Create ();


/*
Once these objects are created, we create a channel object and
associate it to our PHY layer object manager to make sure that all the PHY layer objects created by the
YansWifiPhyHelper share the same underlying channel, that is, they share the same wireless medium and can
communication and interfere:
*/

YansWifiPhyHelper wifiPhyHelper = YansWifiPhyHelper::Default ();
//wifiPhyHelper.SetChannel (wifiChannel);


wifiPhyHelper.SetChannel (wifiChannelHelper.Create ());


wifiPhyHelper.Set ("ShortGuardEnabled", BooleanValue(true));


wifiPhyHelper.Set ("Antennas", UintegerValue (4));
wifiPhyHelper.Set ("MaxSupportedTxSpatialStreams", UintegerValue (4));
wifiPhyHelper.Set ("MaxSupportedRxSpatialStreams", UintegerValue (4));





//Once the PHY helper is configured, we can focus on the MAC layer.
/*
The SetRemoteStationManager method tells the helper the type of rate control algorithm to use. Here, it is
asking the helper to use the AARF algorithm
*/



WifiHelper wifi;// = WifiHelper::Default ();
wifi.SetStandard (WIFI_PHY_STANDARD_80211ac);
wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager", "DataMode", StringValue("VhtMcs9"), "ControlMode", StringValue("VhtMcs0"));
WifiMacHelper mac;// = VhtWifiMacHelper::Default ();







/*
Next, we configure the type of MAC, the SSID of the infrastructure network we want to setup.
This code first creates an 802.11 service set identifier (SSID) object that will be used to set the value of the “Ssid”
Attribute of the MAC layer implementation.

*/
  Ssid ssid = Ssid ("ns3-80211ac");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));


/*
Once all the station-specific parameters are fully configured, both at the MAC and PHY layers, we can invoke our
Install method to create the Wi-Fi devices of these stations:
*/

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (wifiPhyHelper, mac, wifiStaNodes);



/*
We have configured Wi-Fi for all of our STA nodes, and now we need to configure the AP (access point) node. We
begin this process by changing the default Attributes of the WifiMacHelper to reflect the requirements of the
AP. In this case, the WifiMacHelper is going to create MAC layers of the “ns3::ApWifiMac”, the latter specifying that
a MAC instance configured as an AP should be created.
*/

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (wifiPhyHelper, mac, wifiApNode);




//Once install is done, we overwrite the channel width value
Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (160));



//set mobility - all nodes constant

MobilityHelper mobility;

Ptr<PositionAllocator> positionAlloc = CreateObject<RandomBoxPositionAllocator> ();

Ptr<UniformRandomVariable> xVal = CreateObject<UniformRandomVariable> ();
xVal->SetAttribute ("Min", DoubleValue (3));
xVal->SetAttribute ("Max", DoubleValue (10));
positionAlloc->SetAttribute ("X", PointerValue (xVal));

Ptr<UniformRandomVariable> yVal = CreateObject<UniformRandomVariable> ();
yVal->SetAttribute ("Min", DoubleValue (3));
yVal->SetAttribute ("Max", DoubleValue (10));
positionAlloc->SetAttribute ("Y", PointerValue (yVal));

Ptr<UniformRandomVariable> zVal = CreateObject<UniformRandomVariable> ();
zVal->SetAttribute ("Min", DoubleValue (3));
zVal->SetAttribute ("Max", DoubleValue (10));
positionAlloc->SetAttribute ("Z", PointerValue (zVal));

mobility.SetPositionAllocator (positionAlloc);
mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
mobility.Install (wifiStaNodes);






               Ptr<ListPositionAllocator> apPositionAlloc = CreateObject<ListPositionAllocator> ();
 
               apPositionAlloc->Add (Vector (0.0, 0.0, 0.0));

               mobility.SetPositionAllocator (apPositionAlloc);
 
               mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
 
               mobility.Install (wifiApNode);




//The InternetStackHelper is a topology helper that is to internet stacks what the PointToPointHelper is
//to point-to-point net devices. The Install method takes a NodeContainer as a parameter. When it is executed,
//it will install an Internet Stack (TCP, UDP, IP, etc.) on each of the nodes in the node container.
  InternetStackHelper stack;
  stack.Install (csmaNodes);
  stack.Install (wifiApNode);
  stack.Install (wifiStaNodes);


//declare an address helper object and tell it that it should begin allocating IP addresses from the network 10.1.1.0 using
//the mask 255.255.255.0 to define the allocatable bits. By default the addresses allocated will start at one and increase
//monotonically.
/*
First we use the network 10.1.1.0 to create the two addresses needed for our two point-to-point
devices. Then we use network 10.1.2.0 to assign addresses to the CSMA network and then we assign addresses from
network 10.1.3.0 to both the STA devices and the AP on the wireless network.
*/

  Ipv4AddressHelper address;

  address.SetBase ("10.1.1.0", "255.255.255.0");

//performs the actual address assignment. In ns-3 we make the association between an IP address and a device using an
//Ipv4Interface object
  Ipv4InterfaceContainer p2pInterfaces;
  p2pInterfaces = address.Assign (p2pDevices);

  address.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer csmaInterfaces;
  csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  Ipv4InterfaceContainer wifiInterfaces;
  wifiInterfaces =  address.Assign (staDevices);
  address.Assign (apDevices);





/////////////////////////////***TCP*****////////////////////////////////////////////


//Configuration A - 655360 Bytes
//Configuration A
//Configuration A
   
    uint32_t maxBytes = 655360;
    //
    // Create a BulkSendApplication and install it on node 0
    //
    uint16_t bulkPort = 20;  // well-known echo port number
     
    //
    // Create a PacketSinkApplication and install it on node 1
    //
    PacketSinkHelper tcpBulkSink ("ns3::TcpSocketFactory",
                           InetSocketAddress (Ipv4Address::GetAny (), bulkPort));
    ApplicationContainer sinkApps = tcpBulkSink.Install (csmaNodes.Get (0));
    sinkApps.Start (Seconds (1.0));
    sinkApps.Stop (Seconds (10.0));




    BulkSendHelper source ("ns3::TcpSocketFactory",
                           InetSocketAddress (csmaInterfaces.GetAddress (0), bulkPort));
    // Set the amount of data to send in bytes.  Zero is unlimited.
    source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));

for(uint32_t i = 0; i < nWifi; i++) {

    ApplicationContainer sourceApps = source.Install (wifiStaNodes.Get (i));
    sourceApps.Start (Seconds (2.0));
    sourceApps.Stop (Seconds (10.0));
}





//Since we have built an internetwork here, we need to enable internetwork routing

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();






/*
The ns-3 device helpers can also be used to create trace files in the .pcap format. The acronym pcap
stands for packet capture, and is actually an API that includes the definition of a .pcap file format. The
most popular program that can read and display this format is Wireshark. However, there
are many traffic trace analyzers that use this packet format. we can view pcap traces with tcpdump.
*/
  if (tracing == true)
    {
      pointToPoint.EnablePcapAll ("log");
      wifiPhyHelper.EnablePcap ("log", apDevices.Get (0));
      csma.EnablePcap ("log", csmaDevices.Get (0), true);
    }



//ascii tracing
   AsciiTraceHelper ascii;
   Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("ascii-log.tr");
   pointToPoint.EnableAsciiAll (stream);
   csma.EnableAsciiAll (stream);
   wifiPhyHelper.EnableAsciiAll (stream);
   stack.EnableAsciiIpv4All (stream);





//events are actually scheduled in the simulator at 1.0 seconds, 2.0 seconds and two events at 10.0 seconds. When
//Simulator::Run is called, the system will begin looking through the list of scheduled events and executing them.
//First it will run the event at 1.0 seconds, which will enable the echo server application (this event may, in turn, schedule
//many other events). Then it will run the event scheduled for t=2.0 seconds which will start the echo client application.

//Install flow monitor
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.InstallAll();


//The statement will schedule an explicit stop of the simulation at 10 seconds:
  Simulator::Stop (Seconds (10.0));
  
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();


///Flow Monitor

  // Print per flow statistics
   monitor->CheckForLostPackets ();
   Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
   std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats ();

   for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator iter = stats.begin (); iter != stats.end (); ++iter)
     {
 	  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (iter->first);


       for(uint32_t j = 0; j < nWifi; j++) { 

       if (t.sourceAddress == wifiInterfaces.GetAddress(j) && t.destinationAddress == Ipv4Address("10.1.2.1"))
         {
     	  NS_LOG_UNCOND("Flow ID: " << iter->first << " Src Addr " << t.sourceAddress << " Dst Addr " << t.destinationAddress);
     	  NS_LOG_UNCOND("Tx Packets = " << iter->second.txPackets);
     	  NS_LOG_UNCOND("Rx Packets = " << iter->second.rxPackets);
     	  NS_LOG_UNCOND("Throughput: " << iter->second.rxBytes * 8.0 / (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) / 1024  << " Kbps");
          NS_LOG_UNCOND("Total Delay: " << (iter->second.timeLastRxPacket.GetSeconds()-iter->second.timeFirstTxPacket.GetSeconds()) << "seconds");
          NS_LOG_UNCOND("Time-first packet: " << (iter->second.timeFirstTxPacket.GetSeconds()) << "seconds");
          NS_LOG_UNCOND("Time-last packet: " << (iter->second.timeLastRxPacket.GetSeconds()) << "seconds");
         }
      }
     }
   monitor->SerializeToXmlFile("wifi-sim.flowmon", true, true);



  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
  return 0;
}

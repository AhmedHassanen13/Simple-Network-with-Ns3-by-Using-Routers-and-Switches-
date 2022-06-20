#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/ipv4-global-routing-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("project Example");

int
main (int argc, char *argv[])

{
  bool verbose = true;
  uint32_t lan_1 = 2 ;
  uint32_t lan_2 = 2 ;
  uint32_t nWifi = 3 ;

// Code to USE Command Line to run code using cmd

  CommandLine cmd;
  cmd.AddValue ("lan_1", "Number of LAN 1 devices", lan_1);              // Number of devices connected in LAN 1
  cmd.AddValue ("lan_2", "Number of LAN 2 devices", lan_2);             // Number of devices connected in LAN 2
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);         // Number of Wi-fi devices
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);

  cmd.Parse (argc, argv);
  Time::SetResolution (Time::NS);   // Set time resolution to be in nano Seconds

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.

  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  // Type of logs after running code
  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

// Create Nodes
  NodeContainer lan_1_Nodes;   // Nodes in LAN 1
  lan_1_Nodes.Create (lan_1);

  NodeContainer lan_2_Nodes;   // Nodes in LAN 2
  lan_2_Nodes.Create (lan_2);

  NodeContainer router_Nodes;
  router_Nodes.Create (nWifi);


  // Setup channel of LAN 1

  // LAN 1 creating by attaching a CsmaNetDevice to all the nodes on the LAN

  CsmaHelper csma_lan_1;

  csma_lan_1.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));     // Data rate of Channel

  csma_lan_1.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560))); // Delay of packets

  //adding router 1 to list of nodes to connect this with LAN

  lan_1_Nodes.Add (router_Nodes.Get (0));

  // setup channel characteristics in LAN 1 devices

  NetDeviceContainer csma_lan_1_Devices;

  csma_lan_1_Devices = csma_lan_1.Install (lan_1_Nodes);



   // Setup channel of LAN 2

  // LAN 2 creating by attaching a CsmaNetDevice to all the nodes on the LAN

  CsmaHelper csma_lan_2;

  csma_lan_2.SetChannelAttribute ("DataRate", StringValue ("100Mbps"));     // Data rate of Channel

  csma_lan_2.SetChannelAttribute ("Delay", TimeValue (NanoSeconds (6560))); // Delay of packets

  //adding router 1 to list of nodes to connect this with LAN

  lan_2_Nodes.Add (router_Nodes.Get (2));

  // setup channel characteristics in LAN 1 devices

  NetDeviceContainer csma_lan_2_Devices;

  csma_lan_2_Devices = csma_lan_2.Install (lan_2_Nodes);



  //A PointToPoint connection between the three routers

  PointToPointHelper pointToPoint;

  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("10Mbps"));   // Data rate of Channel

  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));       // Delay of packets



  // Setup connection between two routers r1 and r2

  NetDeviceContainer LAN_1_connection;

  LAN_1_connection = pointToPoint.Install (router_Nodes.Get(0), router_Nodes.Get(1));



  // Setup connection between two routers r2 and r3

  NetDeviceContainer LAN_2_connection;

  LAN_2_connection = pointToPoint.Install (router_Nodes.Get(1), router_Nodes.Get(2));



  //Setting IP addresses. Notice that router 1 & 2 are in LAN 1 & 2 respectively.

  InternetStackHelper stack;

  stack.Install (lan_1_Nodes);

  stack.Install (lan_2_Nodes);

  stack.Install (router_Nodes.Get(1));

  Ipv4AddressHelper address;



  //For LAN 1

  address.SetBase ("192.168.1.0", "255.255.255.0");

  Ipv4InterfaceContainer lan_1_interfaces;

  lan_1_interfaces = address.Assign (csma_lan_1_Devices); // get addresses to devices in lan



  //For LAN 2

  address.SetBase ("192.168.2.0", "255.255.255.0");

  Ipv4InterfaceContainer lan_2_interfaces;

  lan_2_interfaces = address.Assign (csma_lan_2_Devices); // get addresses to devices in lan



  //For PointToPoint

  address.SetBase ("172.168.100.0", "255.255.255.0");

  Ipv4InterfaceContainer routerInterfaces;

  routerInterfaces = address.Assign (LAN_1_connection);  // get addresses to devices in lan



  address.SetBase ("172.168.200.0", "255.255.255.0");

  Ipv4InterfaceContainer routerInterfaces2;

  routerInterfaces = address.Assign (LAN_2_connection); // get addresses to devices in lan



  //install a UdpEchoServer on Router 2

  UdpEchoServerHelper echoServer (9);     // port 9 read UdpEchoServer from Server called echoServer

  ApplicationContainer serverApps = echoServer.Install (router_Nodes.Get(1)); // Make router 2 act as a server

  serverApps.Start (Seconds (0));

  serverApps.Stop (Seconds (10));



//create UdpEchoClients characteristics in all LAN 1 devices

  UdpEchoClientHelper echoClient (lan_1_interfaces.GetAddress (0), 9);

  echoClient.SetAttribute ("MaxPackets", UintegerValue (100));

  echoClient.SetAttribute ("Interval", TimeValue (MilliSeconds (200)));

  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));





 // install UdpEchoClient on LAN 2 devices
  NodeContainer client_Nodes (lan_2_Nodes.Get(0), lan_2_Nodes.Get(1) , lan_1_Nodes.Get(1));



  ApplicationContainer clientApps = echoClient.Install (client_Nodes);

  clientApps.Start (Seconds (1));

  clientApps.Stop (Seconds (10));



  //For routers to be able to forward packets, they need to have routing rules.

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();



  csma_lan_1.EnablePcap("lan_1", LAN_1_connection);

  csma_lan_2.EnablePcap("lan_2", LAN_2_connection);

  pointToPoint.EnablePcapAll("routers");



  Simulator::Run ();

  Simulator::Destroy ();

  return 0;

}

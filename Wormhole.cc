/* *** Wormhole.cc ***
 * Testing to see if I can get a normal wireless node to 
 * use 2 network interfaces as well as a point to point. 
 * In the diagram below,nodes n9 and n10 are normal wireless (STA) nodes.
 * Communication between these nodes is via point to point.
 *
 * Wormhole Attack Simulation with AODV Routing Protocol - Sample Program
 * The objective is to see if the routing protocol changes when the P2P
 * nodes are interjected into the normal wireless network. This mimics
 * the operation of an out-of-band wormhole attack.
 *
 * Author : Harry L. May
 *
 * Network topology
 *
 *        ===================== 	Nodes n9 & n10 are the wormhole nodes
 *       // n4---n5---n6---n7  \\
 *     n9  /                 \   n10
 *        /                   \
 *       n0                   n8
 *        \                   /
 *         \                 /
 *          n1------n2------n3
 *
 *               
 *                    
 *            
 * Each node is in the range of its immediate adjacent.
 * Source Node: n0
 * Destination Node: n8
 * Worm Tunnel: Will be between nodes n9 and n10
 * 
 * Output of this file:
 * 1. Generates selected routing table information and
 * 2. wormhole.xml file for viewing animation in NetAnim.
 *
 * Run using : ./waf --run scratch/Wormhole
 * 
 */
#include "ns3/aodv-module.h"
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/adhoc-wifi-mac.h"
#include "ns3/propagation-delay-model.h"
#include "ns3/node.h"
#include "ns3/string.h"
#include <iostream>
#include <fstream>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Wormhole");

int main (int argc, char *argv[])
{
	// *** Variable section ***
	uint32_t numStaNodes = 9;		// Number of wifi nodes
	// These next variables are for setting positions in NetAnim
	double separation1 = 20;			// node separation - top line
	double separation2 = 25;			// node separation - bottom line
	double line1y = 25;					// y axis - top line (starty - line1y) = 5
	double line2y = 12;					// y axis - bottom line (starty + line2y) = 42
	double startx = 0;						// starting location for x reference
	double starty = 30;					// starting location for y reference
	// Node sizes so you can see them in NetAnim
	double nodeSizeX = 5.0;
	double nodeSizeY = 5.0;
	double W1X, W1Y, W2X, W2Y;
	
	int scenario = 2;			// Variable to set initial position of nodes (NetAnim)

//	LogComponentEnable ("AodvRoutingProtocol", LOG_LEVEL_LOGIC);


  std::string phyMode ("DsssRate1Mbps");

/****************
 * If using PCAP files,  the timing can be set to nanoseconds using following:
 ****************
 */
//	Config::SetDefault("ns3::PcapFileWrapper::NanosecMode",BooleanValue(true));
	
//
// Explicitly create the nodes required by the topology (shown above).
//
	NS_LOG_INFO ("Create nodes.");
	NodeContainer allNodes;
	NodeContainer wifiStaNodes;
	NodeContainer p2pNodes;
//
// ********* Scenario: Set position of wormhole nodes  ***********
// Changing positions is for testing
//
    if (scenario == 1) 
	{
		// This sets the wormhole out of range of any normal nodes
		W1X = startx;
		W1Y = starty - 100;
		W2X = startx+130;
		W2Y = starty - 100;
	}
	else 
	{
		// This sets the wormhole nodes within range of the source and destination
		// nodes. Source: N0 Destination: N8
		W1X = startx - 10;
		W1Y = starty - 10;
		W2X = startx + 110;
		W2Y = starty - 10;
	}
//
//  ******** Creation of nodes ***********
//
// First - create the STA nodes and then add them to the 'allNodes'
	wifiStaNodes.Create(numStaNodes);		// wifiStaNodes
	allNodes.Add (wifiStaNodes);
// Next, create the Yans channel and Yans physical for normal nodes
	YansWifiPhyHelper phy;
	YansWifiChannelHelper channel =YansWifiChannelHelper::Default ();
//*** Now, create the Yans channel and Yans physical for wormhole nodes
	YansWifiPhyHelper phy2;
	YansWifiChannelHelper channel2 =YansWifiChannelHelper::Default ();
// Setup for normal nodes
	phy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
	phy.Set ("TxPowerStart", DoubleValue(10));
	phy.Set ("TxPowerEnd", DoubleValue(10));
	phy.SetChannel (channel.Create ());
	channel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	channel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
	  	  	  	  	  	  	  	    "SystemLoss", DoubleValue(1),
		  	  	  	  	  	  	    "HeightAboveZ", DoubleValue(1.5));
//*** Setup for wormhole nodes
	phy2.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 
	phy2.Set ("TxPowerStart", DoubleValue(10));
	phy2.Set ("TxPowerEnd", DoubleValue(10));
	phy2.SetChannel (channel.Create ());

	channel2.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
	channel2.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
	  	  	  	  	  	  	  	    "SystemLoss", DoubleValue(1),
		  	  	  	  	  	  	    "HeightAboveZ", DoubleValue(2));
// Next comes the MAC layer setup
	WifiMacHelper mac;
	mac.SetType ("ns3::AdhocWifiMac");
// Set up the wifi information
	WifiHelper wifi;
	wifi.SetRemoteStationManager ("ns3::AarfWifiManager");
// Set up the wifi2 information
	WifiHelper wifi2;
	wifi2.SetRemoteStationManager ("ns3::AarfWifiManager");

	p2pNodes.Create (2);
	allNodes.Add (p2pNodes);

	NetDeviceContainer staDevices;

//Note: This line adds all the nodes so they can be on 1 network addr.
	staDevices = wifi.Install (phy, mac, allNodes);

// This sets up the 'wired' and the wireless connection for the
//	wormhole nodes.
	NetDeviceContainer p2pDevices;
// This is the wireless part of the wormhole connection
	p2pDevices = wifi2.Install(phy2,mac, p2pNodes);
	
	PointToPointHelper pointToPoint;
	pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
	pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
// This is the wired part of the wormhole connection
	p2pDevices = pointToPoint.Install (p2pNodes);
// 

//  Enable AODV
  AodvHelper aodv;
  AodvHelper malicious_aodv; 
  
// Set up internet stack
	InternetStackHelper stack;
	stack.SetRoutingHelper (aodv);
	stack.Install (allNodes);

// Set Mobility for all nodes
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject <ListPositionAllocator>();
  for (uint8_t i = 0; i < numStaNodes+2; i++)
  {
		switch (i)
		{
				case 0:
					positionAlloc->Add(Vector(startx,starty,0));
					break;
				case 1 ... 3:
					positionAlloc->Add(Vector(startx+separation2*i,starty+line2y,0));
					break;
				case 4 ... 7:
					positionAlloc->Add(Vector(startx-(i-4)*separation1+20,starty-line1y,0));
					break;
				case 8:
					positionAlloc->Add(Vector(startx+100,starty,0));
					break;
				case 9:			// Point to Point wormhole nodes
					positionAlloc->Add(Vector(W1X, W1Y,0));
					break;
				case 10:			// Point to Point wormhole nodes
					positionAlloc->Add(Vector(W2X, W2Y,0));
					break;
				default:
					std::cout << "The number:(i) '" << i << "' is not in range\n";
					break;
		}
    }

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (allNodes);

  // Set up Addresses
	Ipv4AddressHelper ipv4;
	NS_LOG_INFO ("Assign IP Addresses.");
	ipv4.SetBase ("10.1.2.0", "255.255.255.0");		// This is for P2P
	Ipv4InterfaceContainer p2pInterfaces;
	p2pInterfaces = ipv4.Assign (p2pDevices);
  
	ipv4.SetBase ("10.1.1.0", "255.255.255.0");
	Ipv4InterfaceContainer staInterface;
	staInterface = ipv4.Assign(staDevices);
  
// ****** ApplicationContainer *****
// === UdpServer ===
	UdpServerHelper server (9);
	ApplicationContainer serverApps = server.Install (allNodes.Get (numStaNodes-1));
// === UdpClient ===
	UdpClientHelper client (staInterface.GetAddress (0), 9);
	ApplicationContainer clientApps = client.Install (allNodes);
	client.SetAttribute ("MaxPackets", UintegerValue (10));
	client.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
// === start/stop ===
	serverApps.Start (Seconds (1.0));
	serverApps.Stop (Seconds (10.0));
	clientApps.Start (Seconds (2.0));
	clientApps.Stop (Seconds (10.0));


// Enable PCAP files for selected nodes
	phy.EnablePcap ("scratch/Wormhole_Sta0", staDevices.Get (0));
	phy.EnablePcap ("scratch/Wormhole_Sta8", staDevices.Get (8));
// PCAP for point to point devices
	phy2.EnablePcap("scratch/Wormhole_W1", p2pDevices.Get(0));
	phy2.EnablePcap("scratch/Wormhole_W2", p2pDevices.Get(1));
//  pointToPoint.EnablePcap("scratch/myWormholex_p2p",p2pDevices);

	AsciiTraceHelper ascii;

// Animation: This part is the initial setup of the nodes
  AnimationInterface anim ("scratch/Wormhole_anim-"+std::to_string(scenario)+".xml"); // Mandatory
  for (uint8_t i = 0; i < allNodes.GetN(); i++)
  {
		switch (i)
		{
				case 0:
					AnimationInterface::SetConstantPosition (wifiStaNodes.Get (i), startx,starty);
					break;
				case 1 ... 3:
					AnimationInterface::SetConstantPosition (wifiStaNodes.Get (i), startx+separation2*i,starty+line2y);
					break;
				case 4 ... 7:
					AnimationInterface::SetConstantPosition (wifiStaNodes.Get (i), startx+(i-4)*separation1+20,starty-line1y);
					break;
				case 8:
					AnimationInterface::SetConstantPosition (wifiStaNodes.Get (i), startx+100,starty);
					break;
				case 9:
					AnimationInterface::SetConstantPosition (p2pNodes.Get (i-9), W1X, W1Y);
					break;				
				case 10:
					AnimationInterface::SetConstantPosition (p2pNodes.Get (i-9), W2X, W2Y);
					break;				
				default:
					std::cout << "The number '" << i << "' is not in range\n";
					break;
		}
    }
/*   ++++++ REMINDER ++++
 *	The nodes are identified in the order in which they are created.
 *	Nodes 0 - 8 are the wifiStaNodes
 *	Nodes 9 - 10 are the p2pNodes
 *	Sometimes the attributes need the address (.Get(0)) and
 *	sometimes the attribute just needs the number (9, 10, ...) as
 *	in the assignment of the nodeSize.
 */
  anim.UpdateNodeDescription (p2pNodes.Get (0), "W1");
  anim.UpdateNodeDescription (p2pNodes.Get (1), "W2");
  anim.UpdateNodeColor (p2pNodes.Get (0), 0, 0, 255);
  anim.UpdateNodeColor (p2pNodes.Get (1), 0, 0, 255);
  anim.UpdateNodeSize (9, nodeSizeX, nodeSizeY);
  anim.UpdateNodeSize (10, nodeSizeX, nodeSizeY);
	for (uint32_t i = 0; i < numStaNodes; i++)
	{
		anim.UpdateNodeDescription(wifiStaNodes.Get(i),"N"+std::to_string(i));
		anim.UpdateNodeColor (wifiStaNodes.Get(i), 0,255,0);
		anim.UpdateNodeSize  (i, nodeSizeX, nodeSizeY);
	}
// Set wormhole parameters for names, color, and size

  anim.EnablePacketMetadata(true);
  anim.EnableIpv4RouteTracking ("scratch/Wormhole_rt.xml", Seconds(2.5), Seconds(5), Seconds(0.25)); //Optional

	Ptr<OutputStreamWrapper> routingStream = Create<OutputStreamWrapper> ("scratch/routingtable-Wormhole"+std::to_string(scenario)+".s", std::ios::out);
	aodv.PrintRoutingTableAllAt (Seconds (2.5), routingStream);

//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop (Seconds(10));
  Simulator::Run ();

}


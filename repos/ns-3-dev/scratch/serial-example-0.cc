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

// Network topology
//
//       n0    n1   n2   n3
//       |     |    |    |
//       =================
//              LAN
//
// - UDP flows from n0 to n1 and back
// - DropTail queues
// - Tracing of queues and packet receptions to file "udp-echo.tr"

#include <fstream>
#include "ns3/command-line.h"
#include "ns3/serial-net-device.h"
#include "ns3/applications-module.h"
#include "ns3/internet-module.h"
#include "ns3/queue.h"
#include "ns3/log.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("SerialExample");

int
main (int argc, char *argv[])
{
//
// Users may find it convenient to turn on explicit debugging
// for selected modules; the below lines suggest how to do this
//
#if 1
  LogComponentEnable ("SerialExample", LOG_LEVEL_INFO);
  //LogComponentEnable ("SerialClientApplication", LOG_LEVEL_ALL);
  //LogComponentEnable ("SerialServerApplication", LOG_LEVEL_ALL);
#endif
//
// Allow the user to override any of the defaults and the above Bind() at
// run-time, via command-line arguments
//
  CommandLine cmd;
  cmd.Parse (argc, argv);
//
// Explicitly create the nodes required by the topology (shown above).
//
  NS_LOG_INFO ("Create nodes.");
  NodeContainer nodeSet;
  nodeSet.Create (4);

//
// Explicitly create the channels required by the topology (shown above).
//
  NS_LOG_INFO ("Create channels.");
  ObjectFactory deviceFactory("ns3::SerialNetDevice");
  deviceFactory.Set("Mtu", UintegerValue (1400));

  ObjectFactory queueFactory("ns3::DropTailQueue");

  ObjectFactory channelFactory("ns3::SerialChannel");
  channelFactory.Set("DataRate", DataRateValue (DataRate (5000000)));
  channelFactory.Set("Delay", TimeValue (MilliSeconds (2)));
  Ptr< SerialChannel > channel = channelFactory.Create ()->GetObject<SerialChannel> ();

  NS_LOG_INFO ("Create net devices.");
  NetDeviceContainer devs;
  for (NodeContainer::Iterator node = nodeSet.Begin (); node != nodeSet.End (); node++)
    {
	  NS_LOG_INFO ("Create devices");
	  Ptr<SerialNetDevice> device = deviceFactory.Create<SerialNetDevice> ();
	   device->SetAddress (Mac48Address::Allocate ());
	   (*node)->AddDevice (device);
	   Ptr<Queue> queue = queueFactory.Create<Queue> ();
	   device->SetQueue (queue);
	   device->Attach (channel);
       devs.Add (device);
    }

  NS_LOG_INFO ("Assign addresses.");
  Ipv4AddressHelper ipv4;
//
// We've got the "hardware" in place.  Now we need to add IP addresses.
//
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer i = ipv4.Assign (devs);

  NS_LOG_INFO ("Create Applications.");
//
// Create a SerialServer application on node one.
//
  uint16_t port = 9;  // well-known echo port number
  UdpEchoServerHelper server (port);
  // SerialServerHelper server (port);
  ApplicationContainer apps = server.Install (nodeSet.Get (1));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (10.0));

//
// Create a SerialClient application to send UDP datagrams from node zero to
// node one.
//
  uint32_t packetSize = 1024;
  uint32_t maxPacketCount = 1;
  Time interPacketInterval = Seconds (1.);
  UdpEchoClientHelper client (i.GetAddress (1), port);
  // SerialClientHelper client (i.GetAddress (1), port);
  client.SetAttribute ("MaxPackets", UintegerValue (maxPacketCount));
  client.SetAttribute ("Interval", TimeValue (interPacketInterval));
  client.SetAttribute ("PacketSize", UintegerValue (packetSize));
  apps = client.Install (nodeSet.Get (0));
  apps.Start (Seconds (2.0));
  apps.Stop (Seconds (10.0));

#if 1
//
// Users may find it convenient to initialize echo packets with actual data;
// the below lines suggest how to do this
//
  client.SetFill (apps.Get (0), "Hello World");

  client.SetFill (apps.Get (0), 0xa5, 1024);

  uint8_t fill[] = { 0, 1, 2, 3, 4, 5, 6};
  client.SetFill (apps.Get (0), fill, sizeof(fill), 1024);
#endif

  //CsmaHelper csma;
  AsciiTraceHelper ascii;
  //csma.EnableAsciiAll (ascii.CreateFileStream ("udp-echo.tr"));
  //csma.EnablePcapAll ("udp-echo", true);

//
// Now, do the actual simulation.
//
  NS_LOG_INFO ("Run Simulation.");
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("Done.");
}

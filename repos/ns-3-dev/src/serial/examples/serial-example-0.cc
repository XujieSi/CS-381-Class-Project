/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Vanderbilt University
 *
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
 *
 */

#include <iostream>
#include <cmath>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/serial-module.h"
#include "ns3/dsdv-module.h"

using namespace ns3;

uint16_t port = 9;

NS_LOG_COMPONENT_DEFINE ("SerialExample");

class SerialExample
{
public:
  SerialExample ();
  void CaseRun (bool usingWifi,
                double txpDistance);

public:
  uint32_t m_numNodes;
  uint32_t m_nSinks;
  double m_totalTime;
  std::string m_rate;
  std::string m_phyMode;
  uint32_t m_nodeSpeed;
  uint32_t m_periodicUpdateInterval;
  uint32_t m_settlingTime;
  double m_dataStart;
  uint32_t m_bytesTotal;
  uint32_t m_packetsReceived;
  std::string m_csvFileName;
  std::string m_slotFileName;
  uint32_t m_slots;
  uint32_t m_slotTime;
  uint32_t m_gaurdTime;
  uint32_t m_interFrameGap;

private:

  std::map<double, double> m_transmitRangeMap;

  NodeContainer nodes;
  NetDeviceContainer devices;
  Ipv4InterfaceContainer interfaces;

private:
  void CreateNodes ();
  void CreateDevices (std::string tr_name, bool usingWifi, double txpDistance);
  void InstallInternetStack ();
  void InstallApplications ();
  void SetupMobility ();
  void ReceivePacket (Ptr <Socket> );
  Ptr <Socket> SetupPacketReceive (Ipv4Address, Ptr <Node> );
  void CheckThroughput ();
  void InsertIntoTxp (void);

};

int main (int argc, char **argv)
{
	// set the default logging level
	// for example, you can override this by setting by
	// export 'NS_LOG=SerialExampleApplication=level_all|pref_time'

  LogComponentEnable("SerialExample",
			static_cast<LogLevel>(LOG_LEVEL_INFO | LOG_PREFIX_FUNC));
  SerialHelper::EnableLogComponents();

  NS_LOG_INFO ("enter" << argc);
  SerialExample test = SerialExample ();

  test.m_numNodes = 4;
  test.m_nSinks = 1;
  test.m_totalTime = 10.0;
  test.m_rate = "8kbps";
  test.m_phyMode = "DsssRate11Mbps";
  test.m_nodeSpeed = 10; //in m/s
  std::string appl = "all";
  test.m_periodicUpdateInterval = 15;
  test.m_settlingTime = 6;
  test.m_dataStart = 50.0;
  test.m_csvFileName = "SerialExample.csv";
  test.m_slotFileName = "SerialSlots.txt";
  bool usingWifi = false;
  double txpDistance = 400.0;

  // serial parameters
  test.m_slots = test.m_numNodes;
  test.m_slotTime = 1000;
  test.m_interFrameGap = 0;
  test.m_gaurdTime = 0;

  NS_LOG_INFO ("Setting Command Line");

  CommandLine cmd;
  cmd.AddValue ("nWifis", "Number of wifi nodes[Default:30]", test.m_numNodes);
  cmd.AddValue ("nSinks", "Number of wifi sink nodes[Default:10]", test.m_nSinks);
  cmd.AddValue ("usingWifi", "Do you want to use WifiMac(1/0)[Default:false(0)]", usingWifi);
  cmd.AddValue ("totalTime", "Total Simulation time[Default:100]", test.m_totalTime);
  cmd.AddValue ("phyMode", "Wifi Phy mode[Default:DsssRate11Mbps]", test.m_phyMode);
  cmd.AddValue ("rate", "CBR traffic rate[Default:8kbps]", test.m_rate);
  cmd.AddValue ("nodeSpeed", "Node speed in RandomWayPoint model[Default:10]", test.m_nodeSpeed);
  cmd.AddValue ("periodicUpdateInterval", "Periodic Interval Time[Default=15]", test.m_periodicUpdateInterval);
  cmd.AddValue ("settlingTime", "Settling Time before sending out an update for changed metric[Default=6]", test.m_settlingTime);
  cmd.AddValue ("dataStart", "Time at which nodes start to transmit data[Default=50.0]", test.m_dataStart);
  cmd.AddValue ("csvFileName", "The name of the CSV output file name[Default:SerialExample.csv]", test.m_csvFileName);
  cmd.AddValue ("slotFileName", "The name of the input file name[Default:SerialSlots.txt]", test.m_slotFileName);
  cmd.AddValue ("txpDistance", "MaxRange for the node transmissions [Default:400.0]", txpDistance);
  cmd.AddValue ("nSlots", "Number of slots per frame [Default:nWifis]", test.m_slots);
  cmd.AddValue ("slotTime", "Slot transmission Time [Default(us):1000]", test.m_slotTime);
  cmd.AddValue ("gaurdTime", "Duration to wait between slots [Default(us):0]", test.m_gaurdTime);
  cmd.AddValue ("interFrameGap", "Duration between frames [Default(us):0]", test.m_interFrameGap);
  cmd.Parse (argc, argv);

  std::ofstream out (test.m_csvFileName.c_str ());
  out << "SimulationSecond," <<
  "ReceiveRate," <<
  "PacketsReceived," <<
  "NumberOfSinks," <<
  std::endl;
  out.close ();

  SeedManager::SetSeed (12345);

  Config::SetDefault ("ns3::OnOffApplication::PacketSize", StringValue ("1000"));
  Config::SetDefault ("ns3::OnOffApplication::DataRate", StringValue (test.m_rate));

  NS_LOG_INFO ("Running Case");

  test.CaseRun (usingWifi,txpDistance);

  NS_LOG_INFO ("leave");
  return 0;
}

SerialExample::SerialExample ()
  : m_bytesTotal (0),
    m_packetsReceived (0)
{
}

void
SerialExample::ReceivePacket (Ptr <Socket> socket)
{
  NS_LOG_INFO ("enter");
  NS_LOG_UNCOND (Simulator::Now ().GetSeconds () << " Received one packet!");
  Ptr <Packet> packet;
  while (NULL != (packet = socket->Recv ()))
    {
      m_bytesTotal += packet->GetSize ();
      m_packetsReceived += 1;
    }
  NS_LOG_INFO ("leave");
}

void
SerialExample::CheckThroughput ()
{
  NS_LOG_INFO ("enter");
  double kbs = (m_bytesTotal * 8.0) / 1000;
  m_bytesTotal = 0;

  std::ofstream out (m_csvFileName.c_str (), std::ios::app);

  out << Simulator::Now().GetSeconds() << "," << kbs << "," << m_packetsReceived << "," << m_nSinks << std::endl;

  out.close ();
  m_packetsReceived = 0;
  Simulator::Schedule (Seconds (1.0), &SerialExample::CheckThroughput, this);
  NS_LOG_INFO ("leave");
}

Ptr <Socket>
SerialExample::SetupPacketReceive (Ipv4Address addr, Ptr <Node> node)
{
  NS_LOG_INFO ("enter");
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr <Socket> sink = Socket::CreateSocket (node, tid);
  InetSocketAddress local = InetSocketAddress (addr, port);
  sink->Bind (local);
  sink->SetRecvCallback (MakeCallback ( &SerialExample::ReceivePacket, this));
  NS_LOG_INFO ("leave");
  return sink;
}

void
SerialExample::CaseRun (bool usingWifi, double txpDistance)
{
  NS_LOG_INFO ("enter");

  std::stringstream ss;
  ss << m_numNodes;
  std::string t_nodes = ss.str ();

  std::stringstream ss2;
  ss2 << m_totalTime;
  std::string sTotalTime = ss2.str ();

  std::stringstream ss3;
  ss3 << txpDistance;
  std::string t_txpDistance = ss3.str ();

  std::string macProtocol = "";
  if (usingWifi)
    {
      macProtocol = "wifi";
    }
  else
    {
      macProtocol = "serial";
    }

  std::string tr_name = "Dsdv_Manet_"
    + t_nodes + "Nodes_"
    + macProtocol + "_"
    + t_txpDistance + "txDistance_"
    + sTotalTime + "SimTime";
  std::cout << "Trace file generated is " << tr_name << ".tr\n";

  InsertIntoTxp ();
  CreateNodes ();
  CreateDevices (tr_name,usingWifi,txpDistance);
  SetupMobility ();
  InstallInternetStack ();
  InstallApplications ();

  std::cout << "\nStarting simulation for " << m_totalTime << " s ...\n";

  CheckThroughput ();

  Simulator::Stop (Seconds (m_totalTime));
  Simulator::Run ();
  Simulator::Destroy ();
  NS_LOG_INFO ("leave");
}


void
SerialExample::CreateNodes ()
{
  NS_LOG_INFO ("enter");
  std::cout << "Creating " << (unsigned) m_numNodes << " nodes.\n";
  nodes.Create (m_numNodes);
  NS_ASSERT_MSG (m_numNodes > m_nSinks, "Sinks must be less or equal to the number of nodes in network");
  NS_LOG_INFO ("leave");
}

void
SerialExample::SetupMobility ()
{
  NS_LOG_INFO ("ent3er");
  MobilityHelper mobility;
/*
  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", RandomVariableValue (UniformVariable (0, 1000)));
  pos.Set ("Y", RandomVariableValue (UniformVariable (0, 1000)));

  Ptr <PositionAllocator> positionAlloc = pos.Create ()->GetObject <PositionAllocator> ();
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel", "Speed", RandomVariableValue (ConstantVariable (m_nodeSpeed)),
      "Pause", RandomVariableValue (ConstantVariable (2.0)), "PositionAllocator", PointerValue (positionAlloc));
  mobility.SetPositionAllocator (positionAlloc);
*/

  ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", RandomVariableValue (UniformVariable (0.0, 300.0)));
  pos.Set ("Y", RandomVariableValue (UniformVariable (0.0, 0.0)));
  Ptr<PositionAllocator> positionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);
  NS_LOG_INFO ("leave");
}

void
SerialExample::CreateDevices (std::string tr_name, bool usingWifi, double txpDistance)
{
  NS_LOG_INFO ("enter");
  if (usingWifi)
    {
	  NS_LOG_INFO ("using wifi");
      Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (m_phyMode));
      Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2000"));

      double txp;
      if (m_transmitRangeMap.find (txpDistance) == m_transmitRangeMap.end ())
        {
          txp = 0.0;
        }
      else
        {
          txp = m_transmitRangeMap[txpDistance];
        }

      NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
      wifiMac.SetType ("ns3::AdhocWifiMac");
      YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
      YansWifiChannelHelper wifiChannel;
      wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
      wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
      wifiPhy.SetChannel (wifiChannel.Create ());
      WifiHelper wifi;
      wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
      wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                    "DataMode", StringValue (m_phyMode),
                                    "ControlMode", StringValue (m_phyMode));
      //set the tx range based on txpDistance
      wifiPhy.Set ("TxPowerStart", DoubleValue (txp));
      wifiPhy.Set ("TxPowerEnd", DoubleValue (txp));

      devices = wifi.Install (wifiPhy, wifiMac, nodes);

      AsciiTraceHelper ascii;
      wifiPhy.EnableAsciiAll (ascii.CreateFileStream (tr_name + ".tr"));
      wifiPhy.EnablePcapAll (tr_name);
    }
  else
    {
	  NS_LOG_INFO ("using serial");
      Config::SetDefault ("ns3::SerialChannel::MaxRange", DoubleValue (txpDistance));
      // default allocation, each node gets a slot to transmit
      SerialHelper serial = SerialHelper(4,5,
               0,1,1,0,0,0,
               1,0,0,0,0,1,
               2,0,0,1,0,0,
               3,0,0,0,1,0);

      // if SERIAL slot assignment is through a file
      if (this->m_slotFileName.length() > 0) {
    	  SerialHelper wip = SerialHelper (this->m_slotFileName);
    	  if (wip.GetParseState()) {
    		  serial = wip;
    	  }
      } else {
          // in this case selected, numSlots = nodes
    	  // serial = SerialHelper (nodes.GetN (),nodes.GetN ());
      }

      SerialControllerHelper controller;
      controller.Set ("SlotTime", TimeValue (MicroSeconds (1100)));
      controller.Set ("GaurdTime", TimeValue (MicroSeconds (100)));
      controller.Set ("InterFrameTime", TimeValue (MicroSeconds (0)));
      serial.SetSerialControllerHelper (controller);
      devices = serial.Install (nodes);

      AsciiTraceHelper ascii;
      Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream (tr_name + ".tr");
      serial.EnableAsciiAll (stream);
    }
  NS_LOG_INFO ("leave");
}

void
SerialExample::InstallInternetStack ()
{
  NS_LOG_INFO ("enter");
  DsdvHelper dsdv;
  dsdv.Set ("PeriodicUpdateInterval", TimeValue (Seconds (m_periodicUpdateInterval)));
  dsdv.Set ("SettlingTime", TimeValue (Seconds (m_settlingTime)));
  InternetStackHelper stack;
  stack.SetRoutingHelper (dsdv);
  stack.Install (nodes);
  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");
  interfaces = address.Assign (devices);
  NS_LOG_INFO ("leave");
}

void
SerialExample::InstallApplications ()
{
  NS_LOG_INFO ("enter");
  for (uint32_t i = 0; i <= m_nSinks - 1; i++ )
    {
      Ptr<Node> node = NodeList::GetNode (i);
      Ipv4Address nodeAddress = node->GetObject<Ipv4> ()->GetAddress (1, 0).GetLocal ();
      Ptr<Socket> sink = SetupPacketReceive (nodeAddress, node);
    }

  for (uint32_t clientNode = 0; clientNode <= m_numNodes - 1; clientNode++ )
    {
      for (uint32_t j = 0; j <= m_nSinks - 1; j++ )
        {
          OnOffHelper onoff1 ("ns3::UdpSocketFactory", Address (InetSocketAddress (interfaces.GetAddress (j), port)));
          onoff1.SetAttribute ("OnTime", RandomVariableValue (ConstantVariable (1)));
          onoff1.SetAttribute ("OffTime", RandomVariableValue (ConstantVariable (0)));

          if (j != clientNode)
            {
              ApplicationContainer apps1 = onoff1.Install (nodes.Get (clientNode));
              UniformVariable var;
              apps1.Start (Seconds (var.GetValue (m_dataStart, m_dataStart + 1)));
              apps1.Stop (Seconds (m_totalTime));
            }
        }
    }
  NS_LOG_INFO ("leave");
}

/**
 * Set a mapping for transmission power over a particular distance.
 */
void
SerialExample::InsertIntoTxp ()
{
	 NS_LOG_INFO ("enter");
  m_transmitRangeMap.insert (std::pair<double, double> (100.0, 1.2251));
  m_transmitRangeMap.insert (std::pair<double, double> (130.0, 3.5049));
  m_transmitRangeMap.insert (std::pair<double, double> (150.0, 4.7478));
  m_transmitRangeMap.insert (std::pair<double, double> (180.0, 6.3314));
  m_transmitRangeMap.insert (std::pair<double, double> (200.0, 7.2457));
  m_transmitRangeMap.insert (std::pair<double, double> (300.0, 10.7675));
  m_transmitRangeMap.insert (std::pair<double, double> (350.0, 12.1065));
  m_transmitRangeMap.insert (std::pair<double, double> (400.0, 13.2663));
  m_transmitRangeMap.insert (std::pair<double, double> (450.0, 14.2893));
  m_transmitRangeMap.insert (std::pair<double, double> (500.0, 15.2045));
  m_transmitRangeMap.insert (std::pair<double, double> (550.0, 16.0323));
  m_transmitRangeMap.insert (std::pair<double, double> (600.0, 16.7881));
  m_transmitRangeMap.insert (std::pair<double, double> (650.0, 17.4834));
  m_transmitRangeMap.insert (std::pair<double, double> (700.0, 18.1271));
  m_transmitRangeMap.insert (std::pair<double, double> (800.0, 19.2869));
  m_transmitRangeMap.insert (std::pair<double, double> (900.0, 20.3099));
  m_transmitRangeMap.insert (std::pair<double, double> (950.0, 20.7796));
  m_transmitRangeMap.insert (std::pair<double, double> (1000.0, 21.2251));
  m_transmitRangeMap.insert (std::pair<double, double> (27800.0, 50.0));
  NS_LOG_INFO ("leave");
}


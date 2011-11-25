/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Hemanth Narra
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
#include "ns3/test.h"
#include "ns3/simulator.h"
#include "ns3/random-variable.h"
#include "ns3/serial-helper.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/serial-low.h"
#include "ns3/serial-controller.h"
#include "ns3/serial-central.h"
#include "ns3/serial-net-device.h"
#include "ns3/simple-wireless-channel.h"
#include "ns3/names.h"
#include "ns3/callback.h"
#include "ns3/config.h"
#include "ns3/node-container.h"
#include "ns3/mobility-helper.h"

namespace ns3 {
class SerialSlotAllocationTestCase : public TestCase
{
public:
  SerialSlotAllocationTestCase ();
  ~SerialSlotAllocationTestCase ();
  virtual void DoRun (void);
  void MacTxTrace (Ptr<Packet> packet, const WifiMacHeader *hdr);
  void CreateNodes ();
private:
  NodeContainer * m_nodes;
  std::vector<Ptr<Packet> > m_packets;
};

SerialSlotAllocationTestCase::SerialSlotAllocationTestCase (void)
  : TestCase ("Serial slot allocation test case"),
    m_nodes (0)
{
}
SerialSlotAllocationTestCase::~SerialSlotAllocationTestCase ()
{
  delete m_nodes;
  m_packets.clear ();
}
void
SerialSlotAllocationTestCase::DoRun ()
{
  // set the tx range of a node
  Config::SetDefault ("ns3::SimpleWirelessChannel::MaxRange", DoubleValue (303));
  /**
   * CreateNodes and assign mobility models;
   * **required by simple-wireless-channel to identify distance between them**
   */
  CreateNodes ();
  Ptr<SerialController> serialController = CreateObject<SerialController> ();
  serialController->SetSlotTime (MicroSeconds (1100));
  serialController->SetGaurdTime (MicroSeconds (100));
  serialController->SetInterFrameTimeInterval (MicroSeconds (200));
  serialController->SetTotalSlotsAllowed (3);
  Ptr<SimpleWirelessChannel> channel = CreateObject<SimpleWirelessChannel> ();

  //create and initialize node1
  Ptr<SerialNetDevice> device1 = CreateObject<SerialNetDevice> ();
  device1->SetNode (m_nodes->Get (0));
  Ptr<SerialCentralMac> mac1 = CreateObject<SerialCentralMac> ();
  mac1->SetAddress (Mac48Address::Allocate ());
  device1->SetMac (mac1);
  device1->SetSerialController (serialController);
  device1->SetChannel (channel);
  mac1->GetSerialMacLow ()->SetRxCallback (MakeCallback (&SerialSlotAllocationTestCase::MacTxTrace, this));

  Ptr<SerialNetDevice> device2 = CreateObject<SerialNetDevice> ();
  device2->SetNode (m_nodes->Get (1));
  Ptr<SerialCentralMac> mac2 = CreateObject<SerialCentralMac> ();
  mac2->SetAddress (Mac48Address::Allocate ());
  device2->SetMac (mac2);
  device2->SetSerialController (serialController);
  device2->SetChannel (channel);
  mac2->GetSerialMacLow ()->SetRxCallback (MakeCallback (&SerialSlotAllocationTestCase::MacTxTrace, this));

  //creating and enqueueing packets to the macs
  uint32_t pktSize = 1420;
  for (int it = 0; it < 4; it++)
    {
      Ptr<Packet> pkt = Create<Packet> (pktSize);
      m_packets.push_back (pkt);
      pktSize -= 11;
      if (it > 1)
        {
          mac2->Enqueue (pkt,mac1->GetAddress ());
        }
      else
        {
          mac1->Enqueue (pkt,mac1->GetAddress ());
        }
    }

  // setting slots in the SerialController
  serialController->AddSerialSlot (0,mac1);
  serialController->AddSerialSlot (1,mac1);
  serialController->AddSerialSlot (2,mac2);
  serialController->StartSerialSessions ();

  Simulator::Stop (MilliSeconds (8));
  Simulator::Run ();
  Simulator::Destroy ();
  device1->Dispose ();
  device2->Dispose ();
}

void
SerialSlotAllocationTestCase::CreateNodes ()
{
  m_nodes = new NodeContainer;
  m_nodes->Create (2);
  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (303),
                                 "DeltaY", DoubleValue (0),
                                 "GridWidth", UintegerValue (2),
                                 "LayoutType", StringValue ("RowFirst"));
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (*m_nodes);
}

/**
 *  packet Tx times from source:
 *  1420 bytes: 1419*8/11000000 = 0.001032 us
 *  1409 bytes: 1408*8/11000000 = 0.001024 us
 *  1398 bytes: 1397*8/11000000 = 0.001016 us
 *  1387 bytes: 1386*8/11000000 = 0.001008 us
 *
 *  time taken by simple-wireless-channel to move them over the medium
 *  303*3.3*10^(-9) = 1us
 *  node 1 is allotted 2 slots and node 0 is allotted 1 slot
 *  so tx start and transmission slot times for node 0 and node 1 are as follows
 *  Also, GaurdTime is 100 us; so we should add 100 us after every tx slot.
 *          tx start    transmission slot
 *  node 0: 0 us        2200 us (2*1100)
 *  node 1: 2300 us     1100 us (1*1100)
 *
 *  InterframeTime is set as 200 us, thus the next slot times for the nodes are
 *          tx start    transmission slot
 *  node 0: 3700 us     2200 us (2*1100)
 *  node 1: 6000 us     1100 us (1*1100)
 *
 *   so transmit and arrival times of each packet are as follows
 *        tx time     arrival time
 *  pkt0: 0 us        1033 us (0+1032+1)
 *  pkt1: 1033 us     2058 us (1033+1024+1)
 *  pkt2: 2300 us     3317 us (2300+1016+1)
 *  pkt3: 6000 us     7009 us (6000+1008+1)
 *
 */
void
SerialSlotAllocationTestCase::MacTxTrace (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  Time now = Simulator::Now ();
  //validation of received packets based on their sizes
  if (packet->GetSize () == m_packets.at (0)->GetSize ())
    {
      // pkt0
      NS_TEST_ASSERT_MSG_EQ (now.GetMicroSeconds (),1033,"XXX");

    }
  else if (packet->GetSize () == m_packets.at (1)->GetSize ())
    {
      // pkt1
      NS_TEST_ASSERT_MSG_EQ (now.GetMicroSeconds (),2058,"XXX");
    }
  else if (packet->GetSize () == m_packets.at (2)->GetSize ())
    {
      // pkt2
      NS_TEST_ASSERT_MSG_EQ (now.GetMicroSeconds (),3317,"XXX");
    }
  else if (packet->GetSize () == m_packets.at (3)->GetSize ())
    {
      // pkt3
      NS_TEST_ASSERT_MSG_EQ (now.GetMicroSeconds (),7009,"XXX");
    }
  else
    {
      // should not come here
      NS_TEST_ASSERT_MSG_EQ (0,1,"XXX");
    }
}

class SerialTestSuite : public TestSuite
{
public:
  SerialTestSuite () : TestSuite ("serial", SYSTEM)
  {
    AddTestCase (new SerialSlotAllocationTestCase ());
  }
} g_serialTestSuite;
}

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
 * Author: Fred Eisele <phreed@gmail.com>
 *
 */
#include "ns3/assert.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/wifi-mac-header.h"
#include "serial-central.h"

NS_LOG_COMPONENT_DEFINE ("SerialCentralMac");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SerialCentralMac);

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT if (m_low != 0) {std::clog << "[Address=" << m_low->GetAddress () << "] "; }

TypeId
SerialCentralMac::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SerialCentralMac")
    .SetParent<SerialMac> ()
    .AddConstructor<SerialCentralMac> ()
    .AddTraceSource ("MacTx",
                     "A packet has been received from higher layers and is being processed in preparation for "
                     "queueing for transmission.",
                     MakeTraceSourceAccessor (&SerialCentralMac::m_macTxTrace))
    .AddTraceSource ("MacTxDrop",
                     "A packet has been dropped in the MAC layer before being queued for transmission.",
                     MakeTraceSourceAccessor (&SerialCentralMac::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&SerialCentralMac::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx",
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&SerialCentralMac::m_macRxTrace))
    .AddTraceSource ("MacRxDrop",
                     "A packet has been dropped in the MAC layer after it has been passed up from the physical "
                     "layer.",
                     MakeTraceSourceAccessor (&SerialCentralMac::m_macRxDropTrace))
  ;
  return tid;
}

SerialCentralMac::SerialCentralMac ()
{
  NS_LOG_FUNCTION (this);
  m_isSerialRunning = false;
  m_low = CreateObject<SerialMacLow> ();
  m_queue = CreateObject<SerialQueue> ();
  m_queue->SetSerialMacTxDropCallback (MakeCallback (&SerialCentralMac::NotifyTxDrop, this));
}
SerialCentralMac::~SerialCentralMac ()
{
}

void
SerialCentralMac::DoDispose (void)
{
  m_low->Dispose ();
  m_low = 0;
  m_device = 0;
  m_queue = 0;
  m_serialController = 0;
  SerialMac::DoDispose ();
}

void
SerialCentralMac::NotifyTx (Ptr<const Packet> packet)
{
  m_macTxTrace (packet);
}

void
SerialCentralMac::NotifyTxDrop (Ptr<const Packet> packet)
{
  m_macTxDropTrace (packet);
}

void
SerialCentralMac::NotifyRx (Ptr<const Packet> packet)
{
  m_macRxTrace (packet);
}

void
SerialCentralMac::NotifyPromiscRx (Ptr<const Packet> packet)
{
  m_macPromiscRxTrace (packet);
}

void
SerialCentralMac::NotifyRxDrop (Ptr<const Packet> packet)
{
  m_macRxDropTrace (packet);
}

void
SerialCentralMac::SetChannel (Ptr<SerialChannel> channel)
{
  if (channel != 0)
    {
      m_channel = channel;
      m_serialController->SetChannel (channel);
      m_low->SetChannel (m_channel);
    }
}

Ptr<SerialChannel>
SerialCentralMac::GetChannel (void) const
{
  return m_channel;
}

void
SerialCentralMac::SetSerialController (Ptr<SerialController> controller)
{
  m_serialController = controller;
}

Ptr<SerialController>
SerialCentralMac::GetSerialController (void) const
{
  return m_serialController;
}

void
SerialCentralMac::SetDevice (Ptr<SerialNetDevice> device)
{
  m_device = device;
  m_low->SetDevice (m_device);
}

Ptr<SerialNetDevice>
SerialCentralMac::GetDevice (void) const
{
  return m_device;
}

Ptr<SerialMacLow>
SerialCentralMac::GetSerialMacLow (void) const
{
  return m_low;
}

void
SerialCentralMac::SetForwardUpCallback (Callback<void,Ptr<Packet>, Mac48Address, Mac48Address> upCallback)
{
  NS_LOG_FUNCTION (this);
  m_upCallback = upCallback;
}

void
SerialCentralMac::SetLinkUpCallback (Callback<void> linkUp)
{
  linkUp ();
}

void
SerialCentralMac::SetTxQueueStartCallback (Callback<bool,uint32_t> queueStart)
{
  NS_LOG_FUNCTION (this);
  m_queueStart = queueStart;
}

void
SerialCentralMac::SetTxQueueStopCallback (Callback<bool,uint32_t> queueStop)
{
  NS_LOG_FUNCTION (this);
  m_queueStop = queueStop;
}

uint32_t
SerialCentralMac::GetQueueState (uint32_t index)
{
  if (m_queue->GetMaxSize () == m_queue->GetSize ())
    {
      return 0;
    }
  else
    {
      return 1;
    }
}

uint32_t
SerialCentralMac::GetNQueues (void)
{
  //SERIAL currently has only one queue
  return 1;
}

void
SerialCentralMac::SetLinkDownCallback (Callback<void> linkDown)
{
}

void
SerialCentralMac::SetMaxQueueSize (uint32_t size)
{
  NS_LOG_FUNCTION (this << size);
  m_queue->SetMaxSize (size);
}
void
SerialCentralMac::SetMaxQueueDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_queue->SetMaxDelay (delay);
}

Mac48Address
SerialCentralMac::GetAddress (void) const
{
  return m_low->GetAddress ();
}

Ssid
SerialCentralMac::GetSsid (void) const
{
  return m_ssid;
}
void
SerialCentralMac::SetAddress (Mac48Address address)
{
  NS_LOG_FUNCTION (address);
  m_low->SetAddress (address);
  m_low->SetBssid (address);
}
void
SerialCentralMac::SetSsid (Ssid ssid)
{
  NS_LOG_FUNCTION (ssid);
  m_ssid = ssid;
}

Mac48Address
SerialCentralMac::GetBssid (void) const
{
  return m_low->GetBssid ();
}

void
SerialCentralMac::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << from);
  //NotifyRx(packet);
  m_upCallback (packet, from, to);
}

void
SerialCentralMac::Queue (Ptr<const Packet> packet, const WifiMacHeader &hdr)
{
  NS_LOG_FUNCTION (this << packet << &hdr);
  if (!m_queue->Enqueue (packet, hdr))
    {
      NotifyTxDrop (packet);
    }
  //Cannot request for channel access in serial. Serial schedules every node in round robin manner
  //RequestForChannelAccess();
}

void
SerialCentralMac::StartTransmission (uint64_t transmissionTimeUs)
{
  NS_LOG_DEBUG (transmissionTimeUs << " usec");
  Time totalTransmissionSlot = MicroSeconds (transmissionTimeUs);
  if (m_queue->IsEmpty ())
    {
      NS_LOG_DEBUG ("queue empty");
      return;
    }
  WifiMacHeader header;
  Ptr<const Packet> peekPacket = m_queue->Peek (&header);
  Time packetTransmissionTime = m_serialController->CalculateTxTime (peekPacket);
  NS_LOG_DEBUG ("Packet TransmissionTime(microSeconds): " << packetTransmissionTime.GetMicroSeconds () << "usec");
  if (packetTransmissionTime < totalTransmissionSlot)
    {
      totalTransmissionSlot -= packetTransmissionTime;
      Simulator::Schedule (packetTransmissionTime, &SerialCentralMac::SendPacketDown, this,totalTransmissionSlot);
    }
  else
    {
      NS_LOG_DEBUG ("Packet takes more time to transmit than the slot allotted. Will send in next slot");
    }
}

void
SerialCentralMac::SendPacketDown (Time remainingTime)
{
  WifiMacHeader header;
  Ptr<const Packet> packet = m_queue->Dequeue (&header);
  m_low->StartTransmission (packet, &header);
  TxQueueStart (0);
  NotifyTx (packet);
  TxQueueStart (0);
  StartTransmission (remainingTime.GetMicroSeconds ());
}

void
SerialCentralMac::Enqueue (Ptr<const Packet> packet, Mac48Address to, Mac48Address from)
{
  NS_LOG_FUNCTION (this << packet << to << from);
  WifiMacHeader hdr;
  hdr.SetTypeData ();
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (from);
  hdr.SetDsFrom ();
  hdr.SetDsNotTo ();
  Queue (packet, hdr);
}
void
SerialCentralMac::Enqueue (Ptr<const Packet> packet, Mac48Address to)
{
  NS_LOG_FUNCTION (this << packet << to);
  WifiMacHeader hdr;
  hdr.SetTypeData ();
  hdr.SetAddr1 (to);
  hdr.SetAddr2 (GetAddress ());
  hdr.SetAddr3 (m_low->GetAddress ());
  hdr.SetDsFrom ();
  hdr.SetDsNotTo ();
  Queue (packet, hdr);
  NS_LOG_FUNCTION (this << packet << to);
}
bool
SerialCentralMac::SupportsSendFrom (void) const
{
  return true;
}

void
SerialCentralMac::TxOk (const WifiMacHeader &hdr)
{
}
void
SerialCentralMac::TxFailed (const WifiMacHeader &hdr)
{
}

void
SerialCentralMac::TxQueueStart (uint32_t index)
{
  NS_ASSERT (index < GetNQueues ());
  m_queueStart (index);
}
void
SerialCentralMac::TxQueueStop (uint32_t index)
{
  NS_ASSERT (index < GetNQueues ());
  m_queueStop (index);
}

void
SerialCentralMac::Receive (Ptr<Packet> packet, const WifiMacHeader *hdr)
{
  ForwardUp (packet, hdr->GetAddr3 (), hdr->GetAddr1 ());
}

void
SerialCentralMac::DoStart (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_isSerialRunning = true;
  m_queue->SetMacPtr (this);
  m_serialController->Start ();
  m_low->SetRxCallback (MakeCallback (&SerialCentralMac::Receive, this));
  SerialMac::DoStart ();
}

} // namespace ns3

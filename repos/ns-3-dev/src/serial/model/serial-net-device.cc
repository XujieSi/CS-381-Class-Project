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
 * James P.G. Sterbenz <jpgs@ittc.ku.edu>, director
 * ResiliNets Research Group  http://wiki.ittc.ku.edu/resilinets
 * Information and Telecommunication Technology Center (ITTC)
 * and Department of Electrical Engineering and Computer Science
 * The University of Kansas Lawrence, KS USA.
 *
 * Work supported in part by NSF FIND (Future Internet Design) Program
 * under grant CNS-0626918 (Postmodern Internet Architecture),
 * NSF grant CNS-1050226 (Multilayer Network Resilience Analysis and Experimentation on GENI),
 * US Department of Defense (DoD), and ITTC at The University of Kansas.
 */
#include "serial.h"
#include "serial-net-device.h"
#include "ns3/llc-snap-header.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/node.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SerialNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SerialNetDevice);

TypeId
SerialNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SerialNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<SerialNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH),
                   MakeUintegerAccessor (&SerialNetDevice::SetMtu,
                                         &SerialNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> (1,MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH))
    .AddAttribute ("Mac", "The MAC layer attached to this device.",
                   PointerValue (),
                   MakePointerAccessor (&SerialNetDevice::GetMac,
                                        &SerialNetDevice::SetMac),
                   MakePointerChecker<SerialMac> ())
    .AddAttribute ("Channel", "The channel attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&SerialNetDevice::DoGetChannel,
                                        &SerialNetDevice::SetChannel),
                   MakePointerChecker<SerialChannel> ())
    .AddAttribute ("SerialController", "The serial controller attached to this device",
                   PointerValue (),
                   MakePointerAccessor (&SerialNetDevice::GetSerialController,
                                        &SerialNetDevice::SetSerialController),
                   MakePointerChecker<SerialController> ());
  return tid;
}

SerialNetDevice::SerialNetDevice ()
  : m_configComplete (false)
{
  NS_LOG_FUNCTION_NOARGS ();
}
SerialNetDevice::~SerialNetDevice ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
SerialNetDevice::DoDispose (void)
{
  m_node = 0;
  m_mac->Dispose ();
  m_mac = 0;
  m_channel = 0;
  m_serialController = 0;
  // chain up.
  NetDevice::DoDispose ();
}

void
SerialNetDevice::DoStart (void)
{
  m_mac->Start ();
  NetDevice::DoStart ();
}

void
SerialNetDevice::CompleteConfig (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  if (m_mac == 0
      || m_node == 0
      || m_channel == 0
      || m_serialController == 0
      || m_configComplete)
    {
      return;
    }
  //setup mac
  m_mac->SetSerialController (m_serialController);
  m_mac->SetChannel (m_channel);
  // setup callbacks
  m_mac->SetForwardUpCallback (MakeCallback (&SerialNetDevice::ForwardUp, this));
  m_mac->SetLinkUpCallback (MakeCallback (&SerialNetDevice::LinkUp, this));
  m_mac->SetLinkDownCallback (MakeCallback (&SerialNetDevice::LinkDown, this));
  m_mac->SetTxQueueStartCallback (MakeCallback (&SerialNetDevice::TxQueueStart, this));
  m_mac->SetTxQueueStopCallback (MakeCallback (&SerialNetDevice::TxQueueStop, this));
  m_configComplete = true;
}

void
SerialNetDevice::SetMac (Ptr<SerialMac> mac)
{
  m_mac = mac;
  m_mac->SetDevice (this);
  CompleteConfig ();
}
Ptr<SerialMac>
SerialNetDevice::GetMac (void) const
{
  return m_mac;
}

void
SerialNetDevice::SetSerialController (Ptr<SerialController> controller)
{
  m_serialController = controller;
  CompleteConfig ();
}

Ptr<SerialController>
SerialNetDevice::GetSerialController (void) const
{
  return m_serialController;
}

Ptr<Node>
SerialNetDevice::GetNode (void) const
{
  return m_node;
}

void
SerialNetDevice::SetNode (Ptr<Node> node)
{
  m_node = node;
  CompleteConfig ();
}

void
SerialNetDevice::SetChannel (Ptr<SerialChannel> channel)
{
  if (channel != 0)
    {
      m_channel = channel;
      CompleteConfig ();
    }
}

Ptr<Channel>
SerialNetDevice::GetChannel (void) const
{
  return m_channel;
}

Ptr<SerialChannel>
SerialNetDevice::DoGetChannel (void) const
{
  return m_channel;
}

void
SerialNetDevice::SetIfIndex (const uint32_t index)
{
  m_ifIndex = index;
}

uint32_t
SerialNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

void
SerialNetDevice::SetAddress (Address address)
{
  m_mac->SetAddress (Mac48Address::ConvertFrom (address));
}

Address
SerialNetDevice::GetAddress (void) const
{
  return m_mac->GetAddress ();
}

bool
SerialNetDevice::SetMtu (const uint16_t mtu)
{
  if (mtu > MAX_MSDU_SIZE - LLC_SNAP_HEADER_LENGTH)
    {
      return false;
    }
  m_mtu = mtu;
  return true;
}

uint16_t
SerialNetDevice::GetMtu (void) const
{
  return m_mtu;
}

bool
SerialNetDevice::IsLinkUp (void) const
{
  return m_linkUp;
}

void
SerialNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  m_linkChanges.ConnectWithoutContext (callback);
}

bool
SerialNetDevice::IsBroadcast (void) const
{
  return true;
}

Address
SerialNetDevice::GetBroadcast (void) const
{
  return Mac48Address::GetBroadcast ();
}

bool
SerialNetDevice::IsMulticast (void) const
{
  return true;
}

Address
SerialNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  return Mac48Address::GetMulticast (multicastGroup);
}

Address SerialNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool
SerialNetDevice::IsPointToPoint (void) const
{
  return false;
}

bool
SerialNetDevice::IsBridge (void) const
{
  return false;
}

bool
SerialNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (*packet << " Dest:" << dest << " ProtocolNo:" << protocolNumber);
  NS_ASSERT (Mac48Address::IsMatchingType (dest));
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
  // Mac48Address realFrom = Mac48Address::ConvertFrom (GetAddress ());
  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);
  m_mac->Enqueue (packet, realTo);
  return true;
}

bool
SerialNetDevice::NeedsArp (void) const
{
  return true;
}

void
SerialNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_forwardUp = cb;
}

void
SerialNetDevice::ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to)
{
  NS_LOG_FUNCTION (*packet << from << to);
  LlcSnapHeader llc;
  packet->RemoveHeader (llc);
  enum NetDevice::PacketType type;
  if (to.IsBroadcast ())
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_BROADCAST");
      type = NetDevice::PACKET_BROADCAST;
    }
  else if (to.IsGroup ())
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_MULTICAST");
      type = NetDevice::PACKET_MULTICAST;
    }
  else if (to == m_mac->GetAddress ())
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_HOST");
      type = NetDevice::PACKET_HOST;
    }
  else
    {
      NS_LOG_DEBUG ("NetDevice::PACKET_OTHERHOST");
      type = NetDevice::PACKET_OTHERHOST;
    }

  if (type != NetDevice::PACKET_OTHERHOST)
    {
      m_mac->NotifyRx (packet);
      m_forwardUp (this, packet, llc.GetType (), from);
    }

  if (!m_promiscRx.IsNull ())
    {
      m_mac->NotifyPromiscRx (packet);
      m_promiscRx (this, packet, llc.GetType (), from, to, type);
    }
}

void
SerialNetDevice::LinkUp (void)
{
  m_linkUp = true;
  m_linkChanges ();
}

void
SerialNetDevice::LinkDown (void)
{
  m_linkUp = false;
  m_linkChanges ();
}

bool
SerialNetDevice::SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION_NOARGS ();
  NS_ASSERT (Mac48Address::IsMatchingType (dest));
  NS_ASSERT (Mac48Address::IsMatchingType (source));
  Mac48Address realTo = Mac48Address::ConvertFrom (dest);
  Mac48Address realFrom = Mac48Address::ConvertFrom (source);
  LlcSnapHeader llc;
  llc.SetType (protocolNumber);
  packet->AddHeader (llc);
  m_mac->Enqueue (packet, realTo, realFrom);
  return true;
}

void
SerialNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  m_promiscRx = cb;
}

bool
SerialNetDevice::SupportsSendFrom (void) const
{
  return m_mac->SupportsSendFrom ();
}

bool
SerialNetDevice::TxQueueStart (uint32_t index)
{
  m_queueStateChanges (index);
  return true;
}

bool
SerialNetDevice::TxQueueStop (uint32_t index)
{
  m_queueStateChanges (index);
  return true;
}

uint32_t
SerialNetDevice::GetQueueState (uint32_t index)
{
  return m_mac->GetQueueState (index);
}

uint32_t
SerialNetDevice::GetNQueues (void)
{
  return m_mac->GetNQueues ();
}

void
SerialNetDevice::SetQueueStateChangeCallback (Callback<void,uint32_t> callback)
{
  m_queueStateChanges.ConnectWithoutContext (callback);
}

} // namespace ns3


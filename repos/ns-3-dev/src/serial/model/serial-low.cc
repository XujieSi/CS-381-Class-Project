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
#include "ns3/packet.h"
#include "ns3/log.h"
#include "ns3/wifi-mac-trailer.h"
#include "serial-low.h"

NS_LOG_COMPONENT_DEFINE ("SerialMacLow");

#undef NS_LOG_APPEND_CONTEXT
#define NS_LOG_APPEND_CONTEXT std::clog << "[SerialLow=" << m_self << "] "


namespace ns3 {

SerialMacLow::SerialMacLow ()
  : m_currentPacket (0)
{
}

SerialMacLow::~SerialMacLow ()
{
}


void
SerialMacLow::DoDispose (void)
{
  NS_LOG_FUNCTION (this);
  m_channel = 0;
  m_device = 0;
}

void
SerialMacLow::SetAddress (Mac48Address ad)
{
  m_self = ad;
}

void
SerialMacLow::SetBssid (Mac48Address bssid)
{
  m_bssid = bssid;
}
Mac48Address
SerialMacLow::GetAddress (void) const
{
  return m_self;
}

Mac48Address
SerialMacLow::GetBssid (void) const
{
  return m_bssid;
}

void
SerialMacLow::SetRxCallback (Callback<void,Ptr<Packet>,const WifiMacHeader *> callback)
{
  m_rxCallback = callback;
}

Ptr<SerialChannel>
SerialMacLow::GetChannel (void) const
{
  return m_channel;
}

void
SerialMacLow::SetChannel (Ptr<SerialChannel> channel)
{
  NS_LOG_FUNCTION (channel);
  m_channel = channel;
  m_channel->Add (this);
}

Ptr<SerialNetDevice>
SerialMacLow::GetDevice (void) const
{
  return m_device;
}

void
SerialMacLow::SetDevice (Ptr<SerialNetDevice> device)
{
  NS_LOG_FUNCTION (device);
  m_device = device;
}

void
SerialMacLow::StartTransmission (Ptr<const Packet> packet,
                               const WifiMacHeader* hdr)
{
  NS_LOG_FUNCTION (this << packet << hdr);
  m_currentPacket = packet->Copy ();
  m_currentHdr = *hdr;

  NS_LOG_DEBUG ("startTx size=" << GetSize (m_currentPacket, &m_currentHdr) <<
                ", to=" << m_currentHdr.GetAddr1 ());
  m_currentPacket->AddHeader (m_currentHdr);
  WifiMacTrailer fcs;
  m_currentPacket->AddTrailer (fcs);
  ForwardDown (m_currentPacket, &m_currentHdr);
  m_currentPacket = 0;
}

void
SerialMacLow::Receive (Ptr<Packet> packet)
{
  NS_LOG_DEBUG (*packet);
  WifiMacHeader hdr;
  packet->RemoveHeader (hdr);
  if (hdr.IsData () || hdr.IsMgt ())
    {
      NS_LOG_DEBUG ("rx group from=" << hdr.GetAddr2 ());
      WifiMacTrailer fcs;
      packet->RemoveTrailer (fcs);
      m_rxCallback (packet, &hdr);
    }
  else
    {
      NS_LOG_DEBUG ("Need to drop frame as it is not either data or management frame");
    }
  return;
}

uint32_t
SerialMacLow::GetSize (Ptr<const Packet> packet, const WifiMacHeader *hdr) const
{
  WifiMacTrailer fcs;
  return packet->GetSize () + hdr->GetSize () + fcs.GetSerializedSize ();
}

void
SerialMacLow::ForwardDown (Ptr<const Packet> packet, const WifiMacHeader* hdr)
{
  NS_LOG_DEBUG ("send " << hdr->GetTypeString () <<
                ", to=" << hdr->GetAddr1 () <<
                ", size=" << packet->GetSize ());
  //HERE IT IS SIMPLEWIRELESSCHANNEL SEND CALL
  m_channel->Send (packet, this);
}

} // namespace ns3

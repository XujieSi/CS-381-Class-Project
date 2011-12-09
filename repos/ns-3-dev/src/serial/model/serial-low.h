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
#ifndef SERIAL_MAC_LOW_H
#define SERIAL_MAC_LOW_H

#include <vector>
#include <stdint.h>
#include <ostream>
#include <map>

#include "ns3/wifi-mac-header.h"
#include "serial-channel.h"
#include "ns3/data-rate.h"
#include "ns3/mac48-address.h"
#include "ns3/callback.h"
#include "ns3/event-id.h"
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/qos-utils.h"
#include "ns3/node.h"

namespace ns3 {


class SerialMac;
class SerialChannel;
class SerialNetDevice;

class SerialMacLow : public Object
{
public:
  typedef Callback<void, Ptr<Packet>, const WifiMacHeader*> SerialMacLowRxCallback;

  SerialMacLow ();
  virtual ~SerialMacLow ();
  void SetDevice (Ptr<SerialNetDevice> device);
  void SetAddress (Mac48Address ad);
  void SetBssid (Mac48Address ad);
  Mac48Address GetAddress (void) const;
  Mac48Address GetBssid (void) const;
  Ptr<SerialNetDevice> GetDevice (void) const;
  /**
   * Associate the device with a channel
   *
   * \param channel Pointer to the channel
   */
  void SetChannel (Ptr<SerialChannel> channel);
  /**
   * \param callback the callback which receives every incoming packet.
   *
   * This callback typically forwards incoming packets to
   * an instance of ns3::SerialCentralMac.
   */
  void SetRxCallback (Callback<void,Ptr<Packet>,const WifiMacHeader *> callback);
  /**
   * \param packet packet to send
   * \param hdr 802.11 header for packet to send
   *
   * Start the transmission of the input packet and notify the listener
   * of transmission events.
   */
  void StartTransmission (Ptr<const Packet> packet,
                          const WifiMacHeader* hdr);

  /**
   * \param packet packet received
   * \param rxSnr snr of packet received
   * \param txMode transmission mode of packet received
   * \param preamble type of preamble used for the packet received
   *
   * This method is typically invoked by the lower PHY layer to notify
   * the MAC layer that a packet was successfully received.
   */
  void Receive (Ptr<Packet> packet);
private:
  uint32_t GetSize (Ptr<const Packet> packet, const WifiMacHeader *hdr) const;
  void ForwardDown (Ptr<const Packet> packet, const WifiMacHeader *hdr);
  virtual Ptr<SerialChannel> GetChannel (void) const;
  virtual void DoDispose (void);
  SerialMacLowRxCallback m_rxCallback;
  Ptr<Packet> m_currentPacket;
  Ptr<SerialChannel> m_channel;
  Ptr<SerialNetDevice> m_device;
  WifiMacHeader m_currentHdr;
  Mac48Address m_self;
  Mac48Address m_bssid;
};

} // namespace ns3

#endif /* SERIAL_MAC_LOW_H */

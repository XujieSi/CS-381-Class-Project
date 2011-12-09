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
#ifndef SERIAL_NET_DEVICE_H
#define SERIAL_NET_DEVICE_H

#include <string>
#include "ns3/node.h"
#include "ns3/backoff.h"
#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/queue.h"
#include "ns3/ptr.h"
#include "ns3/random-variable.h"
#include "ns3/mac48-address.h"

#include "ns3/serial.h"
#include "ns3/serial-channel.h"
#include "serial-controller.h"

namespace ns3 {

class SerialChannel;
class SerialController;
/**
 * \brief Hold together all Serial-related objects.
 *
 * This class holds together ns3::SerialChannel and
 * ns3::SerialMac
 */
class SerialNetDevice : public NetDevice
{
public:
  static TypeId GetTypeId (void);

  SerialNetDevice ();
  virtual ~SerialNetDevice ();

  /**
   * \param mac the mac layer to use.
   */
  void SetMac (Ptr<SerialMac> mac);
  /**
   * \returns the mac we are currently using.
   */
  Ptr<SerialMac> GetMac (void) const;
  /**
   * \param channel The channel this device is attached to
   */
  void SetChannel (Ptr<SerialChannel> channel);
  /**
   * \param controller The serial controller this device is attached to
   */
  void SetSerialController (Ptr<SerialController> controller);

  // inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);
  virtual bool NeedsArp (void) const;
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);

  virtual Address GetMulticast (Ipv6Address addr) const;

  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, uint16_t protocolNumber);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;

  virtual uint32_t GetQueueState (uint32_t index);
  virtual uint32_t GetNQueues (void);
  virtual void SetQueueStateChangeCallback (Callback<void,uint32_t> callback);

  void SetQueue (Ptr<Queue> q);
  bool Attach (Ptr<SerialChannel> ch);

private:
  // This value conforms to the 802.11 specification
  static const uint16_t MAX_MSDU_SIZE = 2304;

  virtual void DoDispose (void);
  virtual void DoStart (void);
  void ForwardUp (Ptr<Packet> packet, Mac48Address from, Mac48Address to);
  void LinkUp (void);
  void LinkDown (void);

  void Setup (void);
  Ptr<SerialChannel> DoGetChannel (void) const;
  Ptr<SerialController> GetSerialController (void) const;
  void CompleteConfig (void);

  /*
     * The Queue which this CsmaNetDevice uses as a packet source.
     * Management of this Queue has been delegated to the CsmaNetDevice
     * and it has the responsibility for deletion.
     * \see class Queue
     * \see class DropTailQueue
     */
  Ptr<Queue> m_queue;

  Ptr<Node> m_node;
  Ptr<SerialMac> m_mac;
  Ptr<SerialChannel> m_channel;
  Ptr<SerialController> m_serialController;
  NetDevice::ReceiveCallback m_forwardUp;
  NetDevice::PromiscReceiveCallback m_promiscRx;

  TracedCallback<Ptr<const Packet>, Mac48Address> m_rxLogger;
  TracedCallback<Ptr<const Packet>, Mac48Address> m_txLogger;

  bool TxQueueStart (uint32_t index);
  bool TxQueueStop (uint32_t index);
  TracedCallback<uint32_t> m_queueStateChanges;

  uint32_t m_ifIndex;
  bool m_linkUp;
  TracedCallback<> m_linkChanges;
  mutable uint16_t m_mtu;
  bool m_configComplete;
};

} // namespace ns3

#endif /* SERIAL_NET_DEVICE_H */

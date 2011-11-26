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
 * Modified by Fred Eisele <phreed@gmail.com> to suit the
 * SERIAL implementation.
 */
#ifndef SERIAL_CHANNEL_H
#define SERIAL_CHANNEL_H

#include <vector>
#include "ns3/channel.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "serial-low.h"
#include "serial-net-device.h"

namespace ns3 {

class SerialMacLow;
class Packet;

/**
 * \ingroup channel
 * \brief A simple channel, for simple things and testing
 */
class SerialChannel : public Channel
{
public:
  typedef std::vector<Ptr<SerialMacLow> > SerialMacLowList;
  static TypeId GetTypeId (void);
  SerialChannel ();

  /**
   * Copy the packet to be received at a time equal to the transmission
   * time plus the propagation delay between sender and all receivers
   * on the channel that are within the range of the sender
   *
   * \param p Pointer to packet
   * \param sender sending NetDevice
   * \param txTime transmission time (seconds)
   */
  void Send (Ptr<const Packet> p, Ptr<SerialMacLow> sender);

  /**
   * Add a device to the channel
   *
   * \param device Device to add
   */
  void Add (Ptr<SerialMacLow> serialMacLow);
  double GetMaxRange (void) const;

  // inherited from ns3::Channel
  virtual uint32_t GetNDevices (void) const;
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;

private:
  SerialMacLowList m_serialMacLowList;
  double m_range;
};

} // namespace ns3

#endif /* SERIAL_CHANNEL_H */

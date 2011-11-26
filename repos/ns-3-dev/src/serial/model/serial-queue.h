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
#ifndef SERIAL_QUEUE_H
#define SERIAL_QUEUE_H

#include <list>
#include "ns3/packet.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/serial.h"
#include "ns3/wifi-mac-header.h"


namespace ns3 {
/**
 * \brief a SERIAL output queue.
 *
 * When a packet is received by the device, to be sent to the channel,
 * it is queued in the internal queue after being tagged with the
 * current time.
 *
 * When a packet is dequeued, the queue checks its time-stamp
 * to verify whether or not it should be dropped. If m_maxDelay has
 * elapsed, it is dropped.
 * Otherwise, it is returned to the caller.
 */
class SerialQueue : public Object
{
public:
  typedef Callback<void, Ptr<const Packet> > SerialMacTxDropCallback;

  static TypeId GetTypeId (void);
  SerialQueue ();
  ~SerialQueue ();

  void SetMacPtr (Ptr<SerialMac> macPtr);
  /**
   * \brief sets max size of a SerialMacQueue
   */
  void SetMaxSize (uint32_t maxSize);
  /**
   * \brief sets max time a packet could stay in SerialMacQueue
   */
  void SetMaxDelay (Time delay);
  /**
   * \brief returns max size of SerialMacQueue. Packets trying to enqueue after
   * reaching max size will be dropped
   */
  uint32_t GetMaxSize (void) const;
  /**
   * \brief returns max time a packet could stay in SerialMacQueue
   */
  Time GetMaxDelay (void) const;

  bool Enqueue (Ptr<const Packet> packet, const WifiMacHeader &hdr);
  /**
   * \brief Dequeues a packet based on the header passed to it
   *
   * \param hdr header to be dequeued from the SerialMacQueue
   */
  Ptr<const Packet> Dequeue (WifiMacHeader *hdr);
  Ptr<const Packet> Peek (WifiMacHeader *hdr);
  /**
   * If exists, removes <i>packet</i> from queue and returns true. Otherwise it
   * takes no effects and return false. Deletion of the packet is
   * performed in linear time (O(n)).
   */
  bool Remove (Ptr<const Packet> packet);
  void SetSerialMacTxDropCallback (Callback<void,Ptr<const Packet> > callback);
  void Flush (void);
  /**
   * \brief returns true is SerialMacQueue is empty
   */
  bool IsEmpty (void);
  /**
   * \brief returns size of SerialMacQueue
   */
  uint32_t GetSize (void);
private:
  struct Item;

  typedef std::list<struct Item> PacketQueue;
  typedef std::list<struct Item>::reverse_iterator PacketQueueRI;
  typedef std::list<struct Item>::iterator PacketQueueI;

  void Cleanup (void);
  Mac48Address GetAddressForPacket (enum WifiMacHeader::AddressType type, PacketQueueI);

  struct Item
  {
    Item (Ptr<const Packet> packet,
          const WifiMacHeader &hdr,
          Time tstamp);
    Ptr<const Packet> packet;
    WifiMacHeader hdr;
    Time tstamp;
  };

  PacketQueue m_queue;
  uint32_t m_size;
  uint32_t m_maxSize;
  Time m_maxDelay;
  uint32_t m_count;
  Ptr<SerialMac> m_macPtr;
  SerialMacTxDropCallback m_txDropCallback;
};

} // namespace ns3

#endif /* AERO_MAC_QUEUE_H */

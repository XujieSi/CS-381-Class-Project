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
#ifndef SERIAL_CONTROLLER_H
#define SERIAL_CONTROLLER_H

#include "ns3/nstime.h"
#include "ns3/event-id.h"
#include "ns3/data-rate.h"
#include "ns3/packet.h"
#include "ns3/serial-channel.h"
#include "ns3/timer.h"
#include <vector>
#include <map>

namespace ns3 {

enum SerialMode
{
  SENTRALIZED = 1,
};

class SerialMac;
class SerialMacLow;
class SerialChannel;

class SerialController : public Object
{
public:
  static TypeId GetTypeId (void);
  SerialController ();
  ~SerialController ();

  typedef std::map<uint32_t,Ptr<SerialMac> > SerialMacPtrMap;

  /**
   * \param slotTime the duration of a slot.
   *
   * It is a bad idea to call this method after RequestAccess or
   * one of the Notify methods has been invoked.
   */
  void SetSlotTime (Time slotTime);
  /**
   */
  void SetGaurdTime (Time gaurdTime);
  /**
   */
  void SetDataRate (DataRate bps);
  /**
   */
  void AddSerialSlot (uint32_t slot, Ptr<SerialMac> macPtr);
  /**
   */
  void SetInterFrameTimeInterval (Time interFrameTime);
  /**
   */
  void SetTotalSlotsAllowed (uint32_t slotsAllowed);
  /**
   */
  Time GetSlotTime (void) const;
  /**
   */
  Time GetGaurdTime (void) const;
  /**
   */
  DataRate GetDataRate (void) const;
  /**
   */
  Time GetInterFrameTimeInterval (void) const;
  /**
   */
  uint32_t GetTotalSlotsAllowed (void) const;
  /**
   * \param duration expected duration of reception
   *
   * Notify the DCF that a packet reception started
   * for the expected duration.
   */
  void NotifyRxStartNow (Time duration);
  /**
   * Notify the DCF that a packet reception was just
   * completed successfully.
   */
  void NotifyRxEndOkNow (void);
  /**
   * Notify the DCF that a packet reception was just
   * completed unsuccessfully.
   */
  void NotifyRxEndErrorNow (void);
  /**
   * \param duration expected duration of transmission
   *
   * Notify the DCF that a packet transmission was
   * just started and is expected to last for the specified
   * duration.
   */
  void NotifyTxStartNow (Time duration);
  Time CalculateTxTime (Ptr<const Packet> packet);
  void StartSerialSessions (void);
  void SetChannel (Ptr<SerialChannel> c);
  virtual void Start (void);
private:
  static Time GetDefaultSlotTime (void);
  static Time GetDefaultGaurdTime (void);
  static DataRate GetDefaultDataRate (void);
  void DoRestartAccessTimeoutIfNeeded (void);
  void AccessTimeout (void);
  void DoGrantAccess (void);
  bool IsBusy (void) const;
  void UpdateFrameLength (void);
  void ScheduleSerialSession (const uint32_t slotNum);
  Ptr<SerialChannel> GetChannel (void) const;

//  Time m_lastRxStart;
//  Time m_lastRxDuration;
//  bool m_lastRxReceivedOk;
//  Time m_lastRxEnd;
//  Time m_lastTxStart;
//  Time m_lastTxDuration;
//  EventId m_accessTimeout;
  DataRate m_bps;
  uint32_t m_slotTime;
  uint32_t m_gaurdTime;
  uint32_t m_serialFrameLength; //total frameLength in microseconds
  uint32_t m_serialInterFrameTime;
  uint32_t m_totalSlotsAllowed;
  bool m_activeEpoch;
  SerialMode m_serialMode;
  SerialMacPtrMap m_slotPtrs;
  Ptr<SerialChannel> m_channel;
};

} // namespace ns3

#endif /* AERO_SERIAL_CONTROLLER_H */

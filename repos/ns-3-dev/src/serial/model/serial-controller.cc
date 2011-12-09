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
#include "ns3/enum.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "serial-central.h"
#include "serial-controller.h"
#include "serial.h"
#include "serial-low.h"
#include "ns3/abort.h"

NS_LOG_COMPONENT_DEFINE ("SerialController");

#define MY_DEBUG(x) \
  NS_LOG_DEBUG (Simulator::Now () << " " << this << " " << x)

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED (SerialController);

Time
SerialController::GetDefaultSlotTime (void)
{
  return MicroSeconds (1100);
}

Time
SerialController::GetDefaultGaurdTime (void)
{
  return MicroSeconds (100);
}

DataRate
SerialController::GetDefaultDataRate (void)
{
  NS_LOG_DEBUG ("Setting default");
  return DataRate ("11000000b/s");
}

/*************************************************************
 * Serial Controller Class Functions
 ************************************************************/
TypeId
SerialController::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3:SerialController")
    .SetParent<Object> ()
    .AddConstructor<SerialController> ()
    .AddAttribute ("DataRate",
                   "The default data rate for point to point links",
                   DataRateValue (GetDefaultDataRate ()),
                   MakeDataRateAccessor (&SerialController::SetDataRate,
                                         &SerialController::GetDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("SlotTime", "The duration of a Slot in microseconds.",
                   TimeValue (GetDefaultSlotTime ()),
                   MakeTimeAccessor (&SerialController::SetSlotTime,
                                     &SerialController::GetSlotTime),
                   MakeTimeChecker ())
    .AddAttribute ("GaurdTime", "GaurdTime between SERIAL slots in microseconds.",
                   TimeValue (GetDefaultGaurdTime ()),
                   MakeTimeAccessor (&SerialController::SetGaurdTime,
                                     &SerialController::GetGaurdTime),
                   MakeTimeChecker ())
    .AddAttribute ("InterFrameTime", "The wait time between consecutive serial frames.",
                   TimeValue (MicroSeconds (0)),
                   MakeTimeAccessor (&SerialController::SetInterFrameTimeInterval,
                                     &SerialController::GetInterFrameTimeInterval),
                   MakeTimeChecker ())
    .AddAttribute ("SerialMode","Serial Mode, Centralized",
                   EnumValue (SENTRALIZED),
                   MakeEnumAccessor (&SerialController::m_serialMode),
                   MakeEnumChecker (SENTRALIZED, "Centralized"));
  return tid;
}

SerialController::SerialController () : m_activeEpoch (false),
                                    m_channel (0)
{
  NS_LOG_FUNCTION (this);
}

SerialController::~SerialController ()
{
  m_channel = 0;
  m_bps = 0;
  m_slotPtrs.clear ();
}

void
SerialController::DoStart (void)
{
  NS_LOG_FUNCTION (this);
  if (!m_activeEpoch)
    {
      m_activeEpoch = true;
      Simulator::Schedule (NanoSeconds (10),&SerialController::StartSerialSessions, this);
    }
}

void
SerialController::StartSerialSessions (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  ScheduleSerialSession (0);
}

void
SerialController::AddSerialSlot (uint32_t slotPos, Ptr<SerialMac> macPtr)
{
  NS_LOG_FUNCTION (slotPos << macPtr);
  std::pair<std::map<uint32_t, Ptr<SerialMac> >::iterator, bool> result =
    m_slotPtrs.insert (std::make_pair (slotPos,macPtr));
  if (result.second == true)
    {
      NS_LOG_DEBUG ("Added mac : " << macPtr << " in slot " << slotPos);
    }
  else
    {
      NS_LOG_WARN ("Could not add mac: " << macPtr << " to slot " << slotPos);
    }
}

void
SerialController::SetSlotTime (Time slotTime)
{
  NS_LOG_FUNCTION (this << slotTime);
  m_slotTime = slotTime.GetMicroSeconds ();
}

Time
SerialController::GetSlotTime (void) const
{
  return MicroSeconds (m_slotTime);
}

void
SerialController::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this << bps);
  m_bps = bps;
}

DataRate
SerialController::GetDataRate (void) const
{
  return m_bps;
}

void
SerialController::SetChannel (Ptr<SerialChannel> c)
{
  NS_LOG_FUNCTION (this << c);
  m_channel = c;
}


Ptr<SerialChannel>
SerialController::GetChannel (void) const
{
  NS_LOG_FUNCTION (this);
  return m_channel;
}

void
SerialController::SetGaurdTime (Time gaurdTime)
{
  NS_LOG_FUNCTION (this << gaurdTime);
  //gaurdTime is based on the SerialChannel's max range
  if (m_channel != 0)
    {
      m_gaurdTime = Seconds (m_channel->GetMaxRange () / 300000000.0).GetMicroSeconds ();
    }
  else
    {
      m_gaurdTime = gaurdTime.GetMicroSeconds ();
    }
}

Time
SerialController::GetGaurdTime (void) const
{
  return MicroSeconds (m_gaurdTime);
}

void
SerialController::SetInterFrameTimeInterval (Time interFrameTime)
{
  NS_LOG_FUNCTION (interFrameTime);
  m_serialInterFrameTime = interFrameTime.GetMicroSeconds ();
}

Time
SerialController::GetInterFrameTimeInterval (void) const
{
  return MicroSeconds (m_serialInterFrameTime);
}

void
SerialController::SetTotalSlotsAllowed (uint32_t slotsAllowed)
{
  m_totalSlotsAllowed = slotsAllowed;
  m_slotPtrs.clear ();
}

uint32_t
SerialController::GetTotalSlotsAllowed (void) const
{
  return m_totalSlotsAllowed;
}

void
SerialController::ScheduleSerialSession (const uint32_t slotNum)
{
  NS_LOG_FUNCTION (slotNum);
  std::map<uint32_t, Ptr<SerialMac> >::iterator it = m_slotPtrs.find (slotNum);
  if (it == m_slotPtrs.end ())
    {
      NS_LOG_WARN ("No MAC ptrs in SERIAL controller");
    }
  uint32_t numOfSlotsAllotted = 1;
  while (1)
    {
      std::map<uint32_t, Ptr<SerialMac> >::iterator j = m_slotPtrs.find (slotNum + numOfSlotsAllotted);
      if (j != m_slotPtrs.end ())
        {
          if (it->second == j->second)
            {
              numOfSlotsAllotted++;
            }
          else
            {
              break;
            }
        }
      else
        {
          break;
        }
    }
  NS_LOG_DEBUG ("Number of slots allotted for this node is: " << numOfSlotsAllotted);
  Time transmissionSlot = MicroSeconds (GetSlotTime ().GetMicroSeconds () * numOfSlotsAllotted);
  Time totalTransmissionTimeUs = GetGaurdTime () + transmissionSlot;
  NS_ASSERT (it->second != NULL);
  it->second->StartTransmission (transmissionSlot.GetMicroSeconds ());
  if ((slotNum + numOfSlotsAllotted) == GetTotalSlotsAllowed ())
    {
      NS_LOG_DEBUG ("Starting over all sessions again");
      Simulator::Schedule ((totalTransmissionTimeUs + GetInterFrameTimeInterval ()), &SerialController::StartSerialSessions, this);
    }
  else
    {
      NS_LOG_DEBUG ("Scheduling next session");
      Simulator::Schedule (totalTransmissionTimeUs, &SerialController::ScheduleSerialSession, this, (slotNum + numOfSlotsAllotted));
    }
}

Time
SerialController::CalculateTxTime (Ptr<const Packet> packet)
{
  NS_LOG_FUNCTION (*packet);
  NS_ASSERT_MSG (packet->GetSize () < 1500,"PacketSize must be less than 1500B, it is: " << packet->GetSize ());
  return Seconds (m_bps.CalculateTxTime (packet->GetSize ()));
}

} // namespace ns3

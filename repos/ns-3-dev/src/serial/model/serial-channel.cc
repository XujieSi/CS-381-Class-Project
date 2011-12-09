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
#include "serial-channel.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/node.h"
#include "ns3/log.h"
#include "ns3/double.h"
#include "ns3/uinteger.h"
#include "ns3/ptr.h"
#include "ns3/net-device.h"
#include "ns3/mobility-model.h"

NS_LOG_COMPONENT_DEFINE ("SerialChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (SerialChannel);

TypeId
SerialChannel::GetTypeId (void)
{
	static TypeId tid = TypeId ("ns3::SerialChannel")
    		.SetParent<Channel> ()
    		.AddConstructor<SerialChannel> ()
    		.AddAttribute ("MaxRange",
    				"Maximum Transmission Range (meters)",
    				DoubleValue (250),
    				MakeDoubleAccessor (&SerialChannel::m_range),
    				MakeDoubleChecker<double> ())
    		.AddAttribute ("DataRate",
					"The transmission data rate to be provided to devices connected to the channel",
					DataRateValue (DataRate (0xffffffff)),
					MakeDataRateAccessor (&SerialChannel::m_bps),
					MakeDataRateChecker ())
			.AddAttribute ("Delay", "Transmission delay through the channel",
					TimeValue (Seconds (0)),
					MakeTimeAccessor (&SerialChannel::m_delay),
					MakeTimeChecker ())
					;
	return tid;
}

SerialChannel::SerialChannel ()
{
}

void
SerialChannel::Send (Ptr<const Packet> p, Ptr<SerialMacLow> sender)
{
	NS_LOG_FUNCTION (p << sender);
	for (SerialMacLowList::const_iterator i = m_serialMacLowList.begin (); i != m_serialMacLowList.end (); ++i)
	{
		Ptr<SerialMacLow> tmp = *i;
		if (tmp->GetDevice () == sender->GetDevice ())
		{
			continue;
		}
		Ptr<MobilityModel> a = sender->GetDevice ()->GetNode ()->GetObject<MobilityModel> ();
		Ptr<MobilityModel> b = tmp->GetDevice ()->GetNode ()->GetObject<MobilityModel> ();
		NS_ASSERT_MSG (a && b, "Error:  nodes must have mobility models");
		double distance = a->GetDistanceFrom (b);
		NS_LOG_DEBUG ("Distance: " << distance << " Max Range: " << m_range);
		if (distance > m_range)
		{
			continue;
		}
		// speed of light is 3.3 ns/meter
		Time propagationTime = NanoSeconds (uint64_t (3.3 * distance));
		NS_LOG_DEBUG ("Node " << sender->GetDevice ()->GetNode ()->GetId () << " sending to node " <<
				tmp->GetDevice ()->GetNode ()->GetId () << " at distance " << distance <<
				" meters; arriving time (ns): " << propagationTime);
		Simulator::ScheduleWithContext (tmp->GetDevice ()->GetNode ()->GetId (),(propagationTime),
				&SerialMacLow::Receive, tmp, p->Copy ());
	}
}

void
SerialChannel::Add (Ptr<SerialMacLow> serialMacLow)
{
	NS_LOG_DEBUG (this << " " << serialMacLow);
	m_serialMacLowList.push_back (serialMacLow);
	NS_LOG_DEBUG ("current m_serialMacLowList size: " << m_serialMacLowList.size ());
}

uint32_t
SerialChannel::GetNDevices (void) const
{
	return m_serialMacLowList.size ();
}
Ptr<NetDevice>
SerialChannel::GetDevice (uint32_t i) const
{
	return m_serialMacLowList[i]->GetDevice ();
}

double
SerialChannel::GetMaxRange (void) const
{
	return m_range;
}

} // namespace ns3

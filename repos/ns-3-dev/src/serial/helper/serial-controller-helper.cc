/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Hemanth Narra
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
 * Author: Hemanth Narra <hemanthnarra222@gmail.com>
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
#include "serial-controller-helper.h"
#include "ns3/serial-controller.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("SerialControllerHelper");

namespace ns3 {

SerialControllerHelper::SerialControllerHelper ()
{
  m_agentFactory.SetTypeId ("ns3:SerialController");
}

SerialControllerHelper::~SerialControllerHelper ()
{

}

SerialControllerHelper*
SerialControllerHelper::Copy (void) const
{
  return new SerialControllerHelper (*this);
}

Ptr<SerialController>
SerialControllerHelper::Create (void) const
{
  Ptr<SerialController> agent = m_agentFactory.Create<SerialController> ();
  return agent;
}

void
SerialControllerHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

}

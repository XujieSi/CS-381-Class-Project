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
#ifndef SERIALCONTROLLERHELPER_H_
#define SERIALCONTROLLERHELPER_H_

#include "ns3/object-factory.h"
#include "ns3/serial-controller.h"


namespace ns3 {
/**
 * \ingroup aodv
 * \brief Helper class that adds AODV routing to nodes.
 */
class SerialControllerHelper
{
public:
  SerialControllerHelper ();
  ~SerialControllerHelper ();

  Ptr<SerialController> Create (void) const;
  /**
   * \param name the name of the attribute to set
   * \param value the value of the attribute to set.
   *
   * This method controls the attributes of ns3::aodv::RoutingProtocol
   */
  void Set (std::string name, const AttributeValue &value);
  SerialControllerHelper* Copy (void) const;

private:
  ObjectFactory m_agentFactory;
};

}
#endif /* SERIALCONTROLLERHELPER_H_ */

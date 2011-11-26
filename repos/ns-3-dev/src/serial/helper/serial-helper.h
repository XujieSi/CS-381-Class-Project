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
#ifndef SERIAL_HELPER_H
#define SERIAL_HELPER_H

#include <string>
#include <stdarg.h>
#include "ns3/attribute.h"
#include "ns3/serial.h"
#include "ns3/serial-low.h"
#include "ns3/object-factory.h"
#include "ns3/node-container.h"
#include "ns3/net-device-container.h"
#include "ns3/trace-helper.h"
#include "ns3/serial-controller-helper.h"
#include "slot-assignment-parser.h"

namespace ns3 {

class SerialMac;
class SerialNetDevice;
class Node;
class SerialController;

/**
 * \brief create MAC objects
 *
 * This base class must be implemented by new MAC implementation which wish to integrate
 * with the \ref ns3::SerialHelper class.
 */
class SerialMacHelper
{
public:
  virtual ~SerialMacHelper ();
  /**
   * \returns a new MAC object.
   *
   * Subclasses must implement this method to allow the ns3::SerialHelper class
   * to create MAC objects from ns3::SerialHelper::Install.
   */
  virtual Ptr<SerialMac> Create (void) const = 0;
};

/**
 * \brief helps to create SerialNetDevice objects and ensures creation of a centralized
 * SerialController which takes care of the slot scheduling
 */
class SerialHelper : public AsciiTraceHelperForDevice
{
public:
  /**
   * Constructor with two attributes
   *
   * \param numNodes number of nodes in the SERIAL frame
   * \param numSlots total number of slots per frame
   */
  SerialHelper (uint32_t numNodes, uint32_t numSlots);
  /**
   * Constructor with filename for slot assignment
   *
   * \param fileName file name that contains the SERIAL slot assignment.
   * Slot file must be of the format specified below. nodeId followed by
   * ':' and then slot assignment with ',' seperating the slots
   * 0:1,1,0,0,0
   * 1:0,0,1,0,0
   * 2:0,0,0,1,0
   * 3:0,0,0,0,1
   */
  SerialHelper (std::string fileName);
  /**
    * \brief used to set the SERIAL slots from the simulation script
    * useful for low number of nodes. If the number of nodes is more, it is
    * advisable to use the external file to set SERIAL slots.
    * For example, assume serial is a SerialHelper object, the slot assignment
    * for 4 nodes can be set as follows. We have a total of 5 slots,
    * out of which first two slots are assigned to node 1 and the other nodes
    * are assigned single slots as shown below
    * serial.SetSlots(4,5,
    *               0,1,1,0,0,0,
    *               1,0,0,1,0,0,
    *               2,0,0,0,1,0,
    *               3,0,0,0,0,1);
    */
  SerialHelper (int numNodes, int numSlots, ...);

  bool GetParseState() {
	  return m_parser->GetParseState();
  }

  ~SerialHelper ();

  /**
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (NodeContainer c) const;
  /**
   * \param mac the MAC helper to create MAC objects
   * \param node the node on which a aero device must be created
   * \param controller the SerialController to add to these devices
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (Ptr<Node> node) const;
  /**
   * \param nodeName the name of node on which a wifi device must be created
   * \returns a device container which contains all the devices created by this method.
   */
  NetDeviceContainer Install (std::string nodeName) const;

  void SetFileName (std::string filename);
  /**
   * \brief Set the SerialController for this TdamHelper class
   */
  void SetSerialControllerHelper (const SerialControllerHelper &controllerHelper);

  /**
   * Helper to enable all SerialNetDevice log components with one statement
   */
  static void EnableLogComponents (void);
private:
  /**
   * \brief Enable ascii trace output on the indicated net device.
   * \internal
   *
   * NetDevice-specific implementation mechanism for hooking the trace and
   * writing to the trace file.
   *
   * \param stream The output stream object to use when logging ascii traces.
   * \param prefix Filename prefix to use for ascii trace files.
   * \param nd Net device for which you want to enable tracing.
   */
  virtual void EnableAsciiInternal (Ptr<OutputStreamWrapper> stream,
                                    std::string prefix,
                                    Ptr<NetDevice> nd,
                                    bool explicitFilename);
  /**
   * \brief print the SERIAL slot assignment for debugging purposes.
   */
  std::string PrintSlotAllotmentArray (void) const;
  /**
   * \brief Assigns a single SERIAL slot in a frame for each node installed
   * by the SERIAL helper.
   * \internal
   * Slot assignment is done in ascending order of the node id of nodes
   * present in the node container that was passed to the SerialHelper
   * install method
   */
  void SetDefaultSlots (void);
  /**
   * \brief used for slot assignment from an external file
   */
  void SetSlots (void);
  /**
   *  \brief memory allocation of the m_slotAllotmentArray when number of
   *  nodes and total number of slots occupied by those nodes is specified
   *  by the user
   */
  void Allocate2D (void);
  /**
   * memory deallocation of the m_slotAllotmentArray during object deletion
   */
  void Deallocate2D (void);
  /**
   * \brief Populate the m_slotArray in the SerialController class with the
   * mac pointers of nodes assigned to those slots
   *
   * \param mac mac pointer of the node assigned to this SERIAL slot
   * \param nodeId node id assigned to this SERIAL slot
   */
  void AssignSerialSlots (Ptr<SerialMac> mac, uint32_t nodeId) const;

  ObjectFactory m_mac;
  Ptr<SerialChannel> m_channel;
  Ptr<SerialController> m_controller;
  const SerialControllerHelper *m_controllerHelper;
  uint32_t **m_slotAllotmentArray;
  uint32_t m_numRows;
  uint32_t m_numCols;
  std::string m_filename;
  Ptr<SerialSlotAssignmentParser> m_parser;
};

} // namespace ns3

#endif /* AERO_HELPER_H */

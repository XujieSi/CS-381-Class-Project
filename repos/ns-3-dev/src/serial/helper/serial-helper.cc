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
#include "serial-helper.h"
#include "ns3/serial-net-device.h"
#include "ns3/serial-channel.h"
#include "ns3/channel.h"
#include "ns3/mobility-model.h"
#include "ns3/log.h"
#include "ns3/config.h"
#include "ns3/simulator.h"
#include "ns3/names.h"

NS_LOG_COMPONENT_DEFINE ("SerialHelper");

namespace ns3 {

static void AsciiMacTxEventWithContext (Ptr<OutputStreamWrapper> stream, std::string context,
                                        Ptr<const Packet> packet)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << context << " "
                        << *packet << std::endl;
}

static void AsciiMacRxOkEventWithContext (Ptr<OutputStreamWrapper> stream, std::string context,
                                          Ptr<const Packet> packet)
{
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << context << " "
                        << *packet << std::endl;
}

static void AsciiMacTxDropEventWithContext (Ptr<OutputStreamWrapper> stream, std::string context,
                                            Ptr<const Packet> packet)
{
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << context << " "
                        << *packet << std::endl;
}

static void AsciiMacTxEventWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  *stream->GetStream () << "t " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

static void AsciiMacRxOkEventWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  *stream->GetStream () << "r " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

static void AsciiMacTxDropEventWithoutContext (Ptr<OutputStreamWrapper> stream, Ptr<const Packet> packet)
{
  *stream->GetStream () << "d " << Simulator::Now ().GetSeconds () << " " << *packet << std::endl;
}

SerialHelper::SerialHelper (uint32_t numNodes, uint32_t numSlots) : m_controller (0),
                                                                m_controllerHelper (0),
                                                                m_slotAllotmentArray (0),
                                                                m_numRows (numNodes),
                                                                m_numCols (numSlots + 1)
{
  NS_LOG_FUNCTION (this << numNodes << numSlots);
  m_mac.SetTypeId ("ns3::SerialCentralMac");
  m_channel = CreateObject<SerialChannel> ();
  Allocate2D ();
  SetDefaultSlots ();
}

SerialHelper::SerialHelper (std::string filename) : m_controller (0),
                                                m_controllerHelper (0),
                                                m_slotAllotmentArray (0),
                                                m_filename (filename)
{
  NS_LOG_FUNCTION (this << m_filename);
  m_mac.SetTypeId ("ns3::SerialCentralMac");
  m_channel = CreateObject<SerialChannel> ();
  m_parser = CreateObject<SerialSlotAssignmentFileParser> (m_filename);
  if (! m_parser->GetParseState()) return;
  m_numRows = m_parser->GetNodeCount ();
  m_numCols = m_parser->GetTotalSlots () + 1; // +1 for nodeid in first column
  Allocate2D ();
  SetSlots ();
}

SerialHelper::SerialHelper (int numNodes, int numSlots, ...)
      : m_controller (0),
        m_controllerHelper (0),
        m_slotAllotmentArray (0),
        m_numRows (numNodes),
        m_numCols (numSlots + 1)
{
  NS_LOG_FUNCTION (this << numNodes << numSlots);
  va_list args;
  va_start (args, numSlots);
  m_mac.SetTypeId ("ns3::SerialCentralMac");
  m_channel = CreateObject<SerialChannel> ();
  Allocate2D ();
  NS_LOG_DEBUG ("Rows:" << m_numRows << " columns: " << m_numCols);
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      NS_LOG_DEBUG ("new row");
      m_slotAllotmentArray[i][0] = va_arg (args, int);
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          NS_LOG_DEBUG ("new column");
          NS_LOG_DEBUG ("before: value at i: " << i << " j: " << j << " is " << m_slotAllotmentArray[i][j]);
          m_slotAllotmentArray[i][j] = va_arg (args, int);
          NS_LOG_DEBUG ("after: value at i: " << i << " j: " << j << " is " << m_slotAllotmentArray[i][j]);
        }
    }
  va_end (args);
  NS_LOG_DEBUG ("Rows:" << m_numRows << " columns: " << m_numCols << PrintSlotAllotmentArray() );
}

SerialHelper::~SerialHelper ()
{
  NS_LOG_FUNCTION (this);
  Deallocate2D ();
}

void
SerialHelper::SetFileName (std::string filename)
{
  m_filename = filename;
}

void
SerialHelper::SetSerialControllerHelper (const SerialControllerHelper &controllerHelper)
{
  NS_LOG_FUNCTION (this);
  delete m_controllerHelper;
  m_controllerHelper = controllerHelper.Copy ();
  m_controller = m_controllerHelper->Create ();
  m_controller->SetTotalSlotsAllowed (m_numCols - 1);
}

void SerialHelper::Deallocate2D (void)
{
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      free (m_slotAllotmentArray[i]);
    }
  free (m_slotAllotmentArray);
}

/**
 * Allocate a 2 dimensional array with the
 * number of rows and columns specified.
 * Initialize it to 0
 */
void SerialHelper::Allocate2D (void)
{
  NS_LOG_FUNCTION (this);
  m_slotAllotmentArray = (uint32_t**)malloc (m_numRows * sizeof(int*));
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      m_slotAllotmentArray[i] = (uint32_t*)malloc (m_numCols * sizeof(int));
    }
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      m_slotAllotmentArray[i][0] = i; //put in nodeId
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          m_slotAllotmentArray[i][j] = 0;
        }
    }
}

/**
 * The number of rows is equal to the number of nodes.
 * The number of columns is one more than the number of slots.
 * If the number of slots exceeds the number of nodes
 * consecutive slots should be used for the same node as
 * much as possible, to give continuous slots for a node.
 */
void
SerialHelper::SetDefaultSlots (void)
{
  NS_LOG_FUNCTION (this);
  int continuousSlots = (m_numCols - 1) / m_numRows;
  NS_LOG_DEBUG ("continuousSlots:" << continuousSlots
           << " m_numRows:" << m_numRows << " m_numCols:" << m_numCols);
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      int tmp = 0;
      while (tmp < continuousSlots)
        {
          tmp++;
          if (continuousSlots * i + tmp > m_numCols)
            {
              break;
            }
          m_slotAllotmentArray[i][continuousSlots * i + tmp] = 1;
        }
    }
  uint32_t remainingSlots = m_numCols - 1 - (continuousSlots * m_numRows);
  NS_LOG_DEBUG ("remainingSlots:" << remainingSlots);
  for (uint32_t i = 0; i < remainingSlots; i++)
    {
      m_slotAllotmentArray[i][continuousSlots * m_numRows + i + 1] = 1;
    }
  NS_LOG_INFO(PrintSlotAllotmentArray ());
}

/**
 * Update the slot allotment array from the
 * arrays produced by the slot assignment parser.
 */
void
SerialHelper::SetSlots (void)
{
  NS_LOG_INFO("was " << PrintSlotAllotmentArray ());
  std::vector<SlotArray> & slotArrays = m_parser->GetSlotArrays ();
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      m_slotAllotmentArray[i][0] = slotArrays[i][0]; //put in nodeId
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          m_slotAllotmentArray[i][j] = slotArrays[i][j];
        }
    }
  NS_LOG_INFO(PrintSlotAllotmentArray ());
}

void
SerialHelper::AssignSerialSlots (Ptr<SerialMac> mac, uint32_t nodeId) const
{
  NS_LOG_FUNCTION (this << mac << nodeId);
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      if (m_slotAllotmentArray[i][0] == nodeId)
        {
          for (uint32_t j = 1; j < m_numCols; j++)
            {
              //validation of the slots
              NS_ASSERT_MSG (((m_slotAllotmentArray[i][j] == 0) || (m_slotAllotmentArray[i][j] == 1)),
                             "Serial slots should be assigned with only 0 or 1");
              for (uint32_t k = 0; k < m_numRows; k++)
                {
                  if (k == i)
                    {
                      continue;
                    }
                  NS_ASSERT_MSG (!(m_slotAllotmentArray[k][j] == 1 && m_slotAllotmentArray[i][j] == 1),
                                 "Slot exclusivity is not maintained");
                }
              if (m_slotAllotmentArray[i][j] == 1)
                {
                  m_controller->AddSerialSlot (j - 1,mac);
                }
            }
        }
    }
}

std::string
SerialHelper::PrintSlotAllotmentArray (void) const
{
  std::stringstream ss;
  ss << "Slot Allotment Array\n";
  for (uint32_t i = 0; i < m_numRows; i++)
    {
      ss << m_slotAllotmentArray[i][0] << " : \t";
      for (uint32_t j = 1; j < m_numCols; j++)
        {
          ss << m_slotAllotmentArray[i][j] << ",\t";
        }
      ss << "\n";
    }
  return ss.str();
}

NetDeviceContainer
SerialHelper::Install (NodeContainer c) const
{
  NS_LOG_FUNCTION (this);
  NetDeviceContainer devices;
  NS_ASSERT (m_controller != 0);
  for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      Ptr<Node> node = *i;
      Ptr<SerialNetDevice> device = CreateObject<SerialNetDevice> ();
      Ptr<SerialMac> mac = m_mac.Create<SerialMac> ();
      mac->SetAddress (Mac48Address::Allocate ());
      AssignSerialSlots (mac,node->GetId ());
      device->SetMac (mac);
      device->SetChannel (m_channel);
      device->SetSerialController (m_controller);
      node->AddDevice (device);
      devices.Add (device);
    }
  return devices;
}

NetDeviceContainer
SerialHelper::Install (Ptr<Node> node) const
{
  NS_LOG_FUNCTION (this << node);
  return Install (NodeContainer (node));
}

NetDeviceContainer
SerialHelper::Install (std::string nodeName) const
{
  NS_LOG_FUNCTION (this << nodeName);
  Ptr<Node> node = Names::Find<Node> (nodeName);
  return Install (NodeContainer (node));
}

void
SerialHelper::EnableLogComponents (void)
{
  LogComponentEnable ("SerialHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialControllerHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialCentralMac", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialMacLow", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialController", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialMacQueue", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialNetDevice", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("SerialSlotAssignmentFileParser", LOG_LEVEL_ALL);
}

void
SerialHelper::EnableAsciiInternal (
  Ptr<OutputStreamWrapper> stream,
  std::string prefix,
  Ptr<NetDevice> nd,
  bool explicitFilename)
{
  Ptr<SerialNetDevice> device = nd->GetObject<SerialNetDevice> ();
  if (device == 0)
    {
      NS_LOG_INFO ("SerialHelper::EnableAsciiInternal(): Device " << device << " not of type ns3::SerialNetDevice");
      return;
    }

  //
  // Our trace sinks are going to use packet printing, so we have to make sure
  // that is turned on.
  //
  Packet::EnablePrinting ();

  uint32_t nodeid = nd->GetNode ()->GetId ();
  uint32_t deviceid = nd->GetIfIndex ();
  std::ostringstream oss;

  //
  // If we are not provided an OutputStreamWrapper, we are expected to create
  // one using the usual trace filename conventions and write our traces
  // without a context since there will be one file per context and therefore
  // the context would be redundant.
  //
  if (stream == 0)
    {
      //
      // Set up an output stream object to deal with private ofstream copy
      // constructor and lifetime issues.  Let the helper decide the actual
      // name of the file given the prefix.
      //
      AsciiTraceHelper asciiTraceHelper;

      std::string filename;
      if (explicitFilename)
        {
          filename = prefix;
        }
      else
        {
          filename = asciiTraceHelper.GetFilenameFromDevice (prefix, device);
        }

      Ptr<OutputStreamWrapper> theStream = asciiTraceHelper.CreateFileStream (filename);
      //
      // We could go poking through the phy and the state looking for the
      // correct trace source, but we can let Config deal with that with
      // some search cost.  Since this is presumably happening at topology
      // creation time, it doesn't seem much of a price to pay.
      //
      oss.str ("");
      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SerialNetDevice/Mac/MacRx";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiMacRxOkEventWithoutContext, theStream));

      oss.str ("");

      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SerialNetDevice/Mac/MacTx";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiMacTxEventWithoutContext, theStream));

      oss.str ("");

      oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SerialNetDevice/Mac/MacTxDrop";
      Config::ConnectWithoutContext (oss.str (), MakeBoundCallback (&AsciiMacTxDropEventWithoutContext, theStream));

      return;
    }

  //
  // If we are provided an OutputStreamWrapper, we are expected to use it, and
  // to provide a context.  We are free to come up with our own context if we
  // want, and use the AsciiTraceHelper Hook*WithContext functions, but for
  // compatibility and simplicity, we just use Config::Connect and let it deal
  // with coming up with a context.
  //
  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SerialNetDevice/Mac/MacRx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiMacRxOkEventWithContext, stream));

  oss.str ("");

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SerialNetDevice/Mac/MacTx";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiMacTxEventWithContext, stream));

  oss.str ("");

  oss << "/NodeList/" << nodeid << "/DeviceList/" << deviceid << "/$ns3::SerialNetDevice/Mac/MacTxDrop";
  Config::Connect (oss.str (), MakeBoundCallback (&AsciiMacTxDropEventWithContext, stream));
}
} // namespace ns3

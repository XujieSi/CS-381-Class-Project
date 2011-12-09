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
#include <fstream>
#include "ns3/log.h"
#include "ns3/uinteger.h"
#include "slot-assignment-parser.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("SerialSlotAssignmentFileParser");

NS_OBJECT_ENSURE_REGISTERED (SerialSlotAssignmentParser);

TypeId SerialSlotAssignmentParser::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::SerialSlotAssignmentFileParser")
    .SetParent<Object> ();
  return tid;
}

SerialSlotAssignmentParser::SerialSlotAssignmentParser (std::string fileName) : m_numRows (0),
                                                                                    m_numCols (0),
                                                                                    m_parseStatus(true)
{
  NS_LOG_FUNCTION (fileName);
  m_fileName = fileName;
  ParseSerialSlotInformation ();
}

SerialSlotAssignmentParser::~SerialSlotAssignmentParser ()
{
}

void
SerialSlotAssignmentParser::ParseSerialSlotInformation ()
{
  NS_LOG_FUNCTION (this);
  std::ifstream topgen;
  topgen.open (m_fileName.c_str ());
  std::string line;
  if (!topgen.is_open ())
    {
      NS_LOG_WARN ("Couldn't open the file " << m_fileName);
      m_parseStatus = false;
      return;
    }
  int numCells = 0;
  while (!topgen.eof ())
    {
      SlotArray word;
      int pos;
      line.clear ();
      //getline (topgen, line);
      topgen >> line;
      std::string nodeId ("");
      std::stringstream stream (line);
      NS_LOG_DEBUG ("line: " << line);
      getline (stream,nodeId,':');
      if (nodeId == "")
        {
          break;
        }
      m_numRows++;
      NS_LOG_DEBUG ("m_numRows:" << m_numRows);
      line.erase (0,nodeId.size () + 1);
      word.push_back (atoi (nodeId.c_str ()));
      NS_LOG_DEBUG ("nodeId: " << nodeId << " line now:" << line);
      while ( (pos = line.find (',')) > 0)
        {
          NS_LOG_DEBUG ("pos:" << pos);
          std::string field = line.substr (0,pos);
          NS_ASSERT_MSG (field == "0" || field == "1", "slots should only be either 0 or 1");
          line = line.substr (pos + 1);
          word.push_back (atoi (field.c_str ()));
          NS_LOG_DEBUG ("field:" << word.back ());
          numCells++;
        }
      NS_ASSERT_MSG (line == "0" || line == "1", "slots should only be either 0 or 1");
      word.push_back (atoi (line.c_str ()));
      NS_LOG_DEBUG ("field:" << word.back ());
      m_slotArray.push_back (word);
      numCells++;
    }
  NS_ASSERT (numCells % m_numRows == 0);
  m_numCols = numCells / m_numRows;
  NS_LOG_DEBUG ("Nodes in SERIAL: " << m_numRows << " slots per frame: " << m_numCols);
}

uint32_t
SerialSlotAssignmentParser::GetNodeCount (void)
{
  return m_numRows;
}

uint32_t
SerialSlotAssignmentParser::GetTotalSlots (void)
{
  return m_numCols;
}

} /* namespace ns3 */

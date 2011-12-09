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
#ifndef __SERIAL_PARSER_H__
#define __SERIAL_PARSER_H__

#include "ns3/object.h"

namespace ns3 {
typedef std::vector<uint32_t> SlotArray;
class SerialHelper;

class SerialSlotAssignmentParser : public Object
{
public:
  static TypeId GetTypeId (void);
  ~SerialSlotAssignmentParser ();
  SerialSlotAssignmentParser (std::string fileName);
  void ParseSerialSlotInformation (void);
  uint32_t GetNodeCount (void);
  uint32_t GetTotalSlots (void);
  std::vector<SlotArray> & GetSlotArrays ()
  {
    return m_slotArray;
  }
  bool GetParseState()
  {
	  return m_parseStatus;
  }

private:
  std::string m_fileName;
  uint32_t m_numRows; /// Equivalent to number of nodes
  uint32_t m_numCols; /// Equivalent to number of slots plus one
  std::vector<SlotArray> m_slotArray;
  bool m_parseStatus;

};
// end namespace ns3
}
#endif // __SERIAL_PARSER_H__

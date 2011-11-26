.. include:: replace.txt

SERIAL Protocol
-----------------

Serial model is a contention free link layer model built on top of 
``ns3::SimpleWirelessChannel`` model.


SERIAL Overview
*****************

Serial access is a contention avoidance medium access protocol. 
The channel bandwidth is shared competitively by all nodes 
in the network.
The bandwidth is partitioned into time slots for dedicated use among those nodes. 
Each node transmits data only during the dedicated time slot allotted to it,
except when acquiring access to another node's slot. 
The transmission slots are of fixed time intervals, but they may be shared. 
Each transmission slot is separated by a guard interval so that the transmissions do not overlap. 
The value of the guard interval is decided by the serial-net-device model. 
It is usually the amount of time it takes for a packet to travel 
the distance specified by the transmission range. 
In this simple serial model, it is assumed that the clocks of the nodes are synchronized,
any deviation from this included in the guard interval as well.

Implementation of Serial
**********************

The ns-3 implementation of serial has a distributed slot controller 
that assigns transmission slots to various nodes in the network.
The  
``ns3::SerialController`` controls the scheduling aspect of the protocol. 
Serial frame processing, creating MAC headers and trailers, and MAC callback mechanisms are handled by 
``ns3::SerialCentralMac``. 
``ns3::SerialMacQueue`` takes care of the packet queuing and dequeuing.


``ns3::SerialCentralMac``
=======================

``ns3::SerialMac`` is the base class from which ``ns3::SerialCentralMac`` is derived. 
The current implementation considers a simple centralized SERIAL. 
However, considering the other possible implementations of 
distributed serial models, we created a common base class 
so that other implementations could be derived from it. 
All the data packets ready for transmission by the node are sent down from IP to ``ns3::SerialCentralMac``. 
``ns3::SerialCentralMac`` upon receiving the packets, enqueues them and waits for 
its turn to transmit. 
As soon as this node gets its turn to transmit, ``ns3::SerialCentralMac`` looks up the 
``ns3::SerialMacQueue`` for any queued packets.
It then iteratively dequeues packets, attaches the MAC headers 
and trailers and them sends them to ``ns3::SimpleWirelessChannel``. 
Before sending them, serialCentralMac calculates the transmission time 
required based on the packet size and data rate. 
It adds up the transmission times of all the packets sent and compares 
it with the **SlotTime** allotted to it by the ``ns3::SerialController``. 
If it could not transmit any more packets in that slot, 
the loop terminates stopping further transmissions. 
Simple-wireless channel forwards the packets to all the nodes which are within the 
**MaxRange** attribute value specified by the user at the start of simulation. 
``ns3::SerialCentralMac`` also takes care of the packets received from simple-wireless channel. 
It removes the attached MAC headers and trailers and forwards the packet to IP.

``ns3::SerialMacQueue``
=====================

Serial maintains a drop-tail queue to store packets received from the 
network layer until it gets its transmission slot. 
The attributes that can me modified for this class are **MacQueueLength** and **MacQueueTime**. 
So all the packets trying to be enqueued after the queue size reaches **MacQueueLength** 
are be dropped and packets stored in the queue for a time-interval longer 
than **MacQueueTime** are also dropped.

``ns3::SerialController``
=======================

``ns3::SerialController`` takes care of all the scheduling aspects of the protocol. 
It initiates the serial sessions and authorizes the nodes to transmit in the slots specified by it. 
The number of slots alloted for transmission along with the slots durations 
are provided to it as attributes specified by the user at start of simulation. 
The list of attributes along with their default values associated 
with ``ns3::SerialController`` are shown below. 
A ``ns3::SerialHelper`` takes all these attributes along with a list on nodes 
and initializes the serialController. 
``ns3::SerialController`` maintains a list of MAC pointers associated with all the nodes. 
Based on the slot assignment provided by the user, this list is 
populated by ``ns3::SerialHelper`` class before the simulation starts. 
The user can provide the slot assignments for nodes either through the simulation script or an external file. 
After the simulation starts, the ``ns3::SerialController`` initiates 
scheduling of serial sessions based on the node ids. 
It calls the ``ns3::SerialCentralMac`` from its list of MAC pointers and 
instructs the node that it could transmit for a particular **SlotTime**. 
As soon as the transmission slot for that node is complete, 
the ``ns3::SerialController`` waits for **GaurdTime** and then calls 
the next node from the list and so on. 
Once all the nodes from the list are assigned a transmission slot, 
the controller waits for **InterFrameTime** before starting with the same procedure again.

+-----------------+---------------------+
| Attribute       | Default Value       |
+-----------------+---------------------+
| DataRate        | 11 mb/s             |
+-----------------+---------------------+
| SlotTime        | 1100 us             |
+-----------------+---------------------+
| GuardTime       | 100 us              |
+-----------------+---------------------+
| InterFrameTime  | 0 us                |
+-----------------+---------------------+
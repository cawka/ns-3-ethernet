/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Andrey Churin
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
 * Author: Andrey Churin <aachurin@gmail.com>
 */

#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/ethernet-header.h"
#include "ns3/ethernet-trailer.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/enum.h"
#include "ns3/boolean.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/trace-source-accessor.h"
#include "ethernet-net-device.h"
#include "ethernet-channel.h"

NS_LOG_COMPONENT_DEFINE ("EthernetNetDevice");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EthernetNetDevice);

TypeId 
EthernetNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EthernetNetDevice")
    .SetParent<NetDevice> ()
    .AddConstructor<EthernetNetDevice> ()
    .AddAttribute ("Address", 
                   "The MAC address of this device.",
                   Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                   MakeMac48AddressAccessor (&EthernetNetDevice::SetMac48Address, 
                                             &EthernetNetDevice::GetMac48Address),
                   MakeMac48AddressChecker ())
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (DEFAULT_MTU),
                   MakeUintegerAccessor (&EthernetNetDevice::SetMtu,
                                         &EthernetNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("EncapsulationMode", 
                   "The link-layer encapsulation type to use.",
                   EnumValue (CsmaNetDevice::DIX),
                   MakeEnumAccessor (&EthernetNetDevice::SetEncapsulationMode),
                   MakeEnumChecker (CsmaNetDevice::DIX, "Dix",
                                    CsmaNetDevice::LLC, "Llc"))
    .AddAttribute ("ReceiveErrorModel", 
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&EthernetNetDevice::SetReceiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("TxQueue", 
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&EthernetNetDevice::SetQueue,
                                        &EthernetNetDevice::GetQueue),
                   MakePointerChecker<Queue> ())
    .AddTraceSource ("MacTx", 
                     "Trace source indicating a packet has arrived for transmission by this device",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_macTxTrace))
    .AddTraceSource ("MacTxDrop", 
                     "Trace source indicating a packet has been dropped by the device before transmission",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_macTxDropTrace))
    .AddTraceSource ("MacPromiscRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_macPromiscRxTrace))
    .AddTraceSource ("MacRx", 
                     "A packet has been received by this device, has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_macRxTrace))
    .AddTraceSource ("PhyTxBegin", 
                     "Trace source indicating a packet has begun transmitting over the channel",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_phyTxBeginTrace))
    .AddTraceSource ("PhyTxEnd", 
                     "Trace source indicating a packet has been completely transmitted over the channel",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_phyTxEndTrace))
    .AddTraceSource ("PhyTxDrop", 
                     "Trace source indicating a packet has been dropped by the device during transmission",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_phyTxDropTrace))
    .AddTraceSource ("PhyRxEnd", 
                     "Trace source indicating a packet has been completely received by the device",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_phyRxEndTrace))
    .AddTraceSource ("PhyRxDrop", 
                     "Trace source indicating a packet has been dropped by the device during reception",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_phyRxDropTrace))
    .AddTraceSource ("PromiscSniffer", 
                     "Trace source simulating a promiscuous packet sniffer attached to the device",
                     MakeTraceSourceAccessor (&EthernetNetDevice::m_promiscSnifferTrace))
    ;
  return tid;
}

ProxyTracedCallback::ProxyTracedCallback (const std::string &name, Ptr<Object> obj1, Ptr<Object> obj2)
  : m_name (name),
    m_obj1 (obj1),
    m_obj2 (obj2)
{
  NS_ASSERT (obj1 != 0);
}

ProxyTracedCallback::~ProxyTracedCallback ()
{
  m_obj1 = 0;
  m_obj2 = 0;
}
  
void
ProxyTracedCallback::ConnectWithoutContext (const CallbackBase &callback)
{
  m_obj1->TraceConnectWithoutContext (m_name, callback);
  if (m_obj2 != 0)
    {
      m_obj2->TraceConnectWithoutContext (m_name, callback);
    }
}

void
ProxyTracedCallback::Connect (const CallbackBase &callback, std::string context)
{
  m_obj1->TraceConnect (m_name, context, callback);
  if (m_obj2 != 0)
    {
      m_obj2->TraceConnect (m_name, context, callback);
    }  
}

void
ProxyTracedCallback::DisconnectWithoutContext (const CallbackBase &callback)
{
  m_obj1->TraceDisconnectWithoutContext (m_name, callback);
  if (m_obj2 != 0)
    {
      m_obj2->TraceDisconnectWithoutContext (m_name, callback);
    }
}

void
ProxyTracedCallback::Disconnect (const CallbackBase &callback, std::string context)
{
  m_obj1->TraceDisconnect (m_name, context, callback);
  if (m_obj2 != 0)
    {
      m_obj2->TraceDisconnect (m_name, context, callback);
    }  
}

EthernetNetDevice::EthernetNetDevice ()
  : m_linkUp (false),
    m_encapMode (CsmaNetDevice::DIX),
    m_node (0),
    m_txDev (CreateObject<CsmaNetDevice> ()),
    m_rxDev (CreateObject<CsmaNetDevice> ()),
    m_macTxTrace ("MacTx", m_txDev),
    m_macTxDropTrace ("MacTxDrop", m_txDev),
    m_macPromiscRxTrace ("MacPromiscRx", m_rxDev),
    m_macRxTrace ("MacRx", m_rxDev),
    m_phyTxBeginTrace ("PhyTxBegin", m_txDev),
    m_phyTxEndTrace ("PhyTxEnd", m_txDev),
    m_phyTxDropTrace ("PhyTxDrop", m_txDev),
    m_phyRxEndTrace ("PhyRxEnd", m_rxDev),
    m_phyRxDropTrace ("PhyRxDrop", m_rxDev),
    m_promiscSnifferTrace ("PromiscSniffer", m_txDev, m_rxDev)
{
  NS_LOG_FUNCTION (this);
}

EthernetNetDevice::~EthernetNetDevice()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void 
EthernetNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  m_txDev->Dispose ();
  m_rxDev->Dispose ();
  m_txDev = 0;
  m_rxDev = 0;
  m_node = 0;
  m_channel = 0;
  NetDevice::DoDispose ();
}

Ptr<Node> 
EthernetNetDevice::GetNode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_node;
}

void 
EthernetNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (node);
  m_node = node;
  m_rxDev->SetNode (node);
  m_txDev->SetNode (node);  
}

bool
EthernetNetDevice::SetMac48Address (Mac48Address address)
{
  NS_LOG_FUNCTION (address);
  m_address = address;
  m_txDev->SetAddress (address);
  m_rxDev->SetAddress (address);
  return true;
}

Mac48Address
EthernetNetDevice::GetMac48Address (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

void 
EthernetNetDevice::SetEncapsulationMode (enum CsmaNetDevice::EncapsulationMode mode)
{
  NS_LOG_FUNCTION (mode);

  m_encapMode = mode;
  m_txDev->SetEncapsulationMode (mode);
}

CsmaNetDevice::EncapsulationMode
EthernetNetDevice::GetEncapsulationMode (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_encapMode;
}
  
bool
EthernetNetDevice::SetMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (mtu);

  m_mtu = mtu;
  m_txDev->SetMtu (mtu);

  return true;
}

uint16_t
EthernetNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_mtu;
}

Ptr<CsmaNetDevice>
EthernetNetDevice::GetTxDevice (void) const
{
  return m_txDev;
}

Ptr<CsmaNetDevice>
EthernetNetDevice::GetRxDevice (void) const
{
  return m_rxDev;
}
  
void
EthernetNetDevice::SetInterframeGap (Time t)
{
  NS_LOG_FUNCTION (t);
  m_txDev->SetInterframeGap (t);
}

bool
EthernetNetDevice::Attach (const Ptr<EthernetChannel> &channel)
{
  NS_LOG_FUNCTION (this << &channel);

  NS_ASSERT (channel != 0);

  m_channel = channel;
  
  m_channel->Attach (this);
    
  NotifyLinkUp ();
  return true;
}

void
EthernetNetDevice::SetQueue (const Ptr<Queue> &queue)
{
  NS_LOG_FUNCTION (queue);
  m_txDev->SetQueue (queue);
}

Ptr<Queue>
EthernetNetDevice::GetQueue (void) const 
{ 
  NS_LOG_FUNCTION_NOARGS ();
  return m_txDev->GetQueue ();
}

void
EthernetNetDevice::SetReceiveErrorModel (const Ptr<ErrorModel> &em)
{
  NS_LOG_FUNCTION (em);
  m_rxDev->SetReceiveErrorModel (em); 
}

void
EthernetNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (index);
  m_ifIndex = index;
}

uint32_t 
EthernetNetDevice::GetIfIndex (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_ifIndex;
}

Ptr<Channel>
EthernetNetDevice::GetChannel (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_channel;
}

void
EthernetNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION_NOARGS ();
  SetMac48Address (Mac48Address::ConvertFrom (address));
}

Address 
EthernetNetDevice::GetAddress (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_address;
}

bool
EthernetNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_linkUp;
}

bool 
EthernetNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
EthernetNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool 
EthernetNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
EthernetNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (multicastGroup);
  return Mac48Address::GetMulticast (multicastGroup);
}

bool 
EthernetNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

bool 
EthernetNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return false;
}

bool
EthernetNetDevice::Send (Ptr<Packet> packet,const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << dest << protocolNumber);
  return m_txDev->Send (packet, dest, protocolNumber);
}

bool
EthernetNetDevice::SendFrom (Ptr<Packet> packet, const Address& src, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (packet << src << dest << protocolNumber);
  return m_txDev->SendFrom (packet, src, dest, protocolNumber);
}

bool 
EthernetNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return true;
}

Address
EthernetNetDevice::GetMulticast (Ipv6Address addr) const
{
  return Mac48Address::GetMulticast (addr);
}

bool 
EthernetNetDevice::SupportsSendFrom () const
{
  return true;
}

void
EthernetNetDevice::NotifyLinkUp (void)
{
  NS_LOG_FUNCTION_NOARGS ();
  m_linkUp = true;
  m_linkChangeCallbacks ();
}

void
EthernetNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  m_linkChangeCallbacks.ConnectWithoutContext (callback);
}

void 
EthernetNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
  if (cb.IsNull ()) 
    {
      m_rxDev->SetReceiveCallback (cb);
    }
  else
    {
      m_rxDev->SetReceiveCallback (MakeCallback (&EthernetNetDevice::NonPromiscReceiveFromDevice, this));
    }
}

void 
EthernetNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  m_promiscRxCallback = cb;
    if (cb.IsNull ()) 
    {
      m_rxDev->SetPromiscReceiveCallback (cb);
    }
  else
    {
      m_rxDev->SetPromiscReceiveCallback (MakeCallback (&EthernetNetDevice::PromiscReceiveFromDevice, this));
    }
}

bool
EthernetNetDevice::NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                   const Address &from)
{
  return m_rxCallback (this, packet, protocol, from);
}

bool
EthernetNetDevice::PromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                const Address &from, const Address &to, NetDevice::PacketType packetType)
{
  return m_promiscRxCallback (this, packet, protocol, from, to, packetType);
}

} // namespace ns3

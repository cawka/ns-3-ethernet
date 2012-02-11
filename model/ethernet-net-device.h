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
 
#ifndef ETHERNET_NET_DEVICE_H
#define ETHERNET_NET_DEVICE_H

#include <string.h>
#include "ns3/node.h"
#include "ns3/address.h"
#include "ns3/net-device.h"
#include "ns3/callback.h"
#include "ns3/packet.h"
#include "ns3/traced-callback.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/ptr.h"
#include "ns3/random-variable.h"
#include "ns3/mac48-address.h"
#include "ns3/csma-net-device.h"

namespace ns3 {

class Queue;
class EthernetChannel;
class ErrorModel;

class ProxyTracedCallback
{
public:
  ProxyTracedCallback (const std::string &name, Ptr<Object> obj1, Ptr<Object> obj2 = 0);
  ~ProxyTracedCallback ();
 
  void ConnectWithoutContext (const CallbackBase &callback);
  void Connect (const CallbackBase &callback, std::string context);
  void DisconnectWithoutContext (const CallbackBase &callback);
  void Disconnect (const CallbackBase &callback, std::string context);
private:
  std::string m_name;
  Ptr<Object> m_obj1;
  Ptr<Object> m_obj2;
};
  
/**
 * \class EthernetNetDevice
 * \brief A Device for a Ethernet Network Link.
 *
 * The Ethernet net device class is analogous to layer 1 and 2 of the
 * TCP stack. The NetDevice takes a raw packet of bytes and creates a
 * protocol specific packet from them.
 */
class EthernetNetDevice : public NetDevice 
{
public:
  static TypeId GetTypeId (void);
  /**
   * Construct a EthernetNetDevice
   *
   * This is the default constructor for a EthernetNetDevice.
   */
  EthernetNetDevice ();
  /**
   * Destroy a EthernetNetDevice
   *
   * This is the destructor for a EthernetNetDevice.
   */
  virtual ~EthernetNetDevice ();
  /**
   * Set Mac48 address
   */
  bool SetMac48Address (Mac48Address address);
  /**
   * Returns Mac48 address
   */  
  Mac48Address GetMac48Address (void) const;
  /**
   * Set the interframe gap used to separate packets.  The interframe gap
   * defines the minimum space required between packets sent by this device.
   * It defaults to 96 bit times.
   *
   * @param t the interframe gap time
   */
  void SetInterframeGap (Time t);
  /**
   * Attach the device to a channel.
   *
   * @param channel EthernetChannel 
   */
  bool Attach (const Ptr<EthernetChannel> &channel);
  /**
   * Attach a queue to the EthernetNetDevice.
   *
   * The EthernetNetDevice "owns" a queue. This queue may be set by higher
   * level topology objects to implement a particular queueing method such as
   * DropTail or RED.  
   *
   * @param queue the queue for being assigned to the device.
   */
  void SetQueue (const Ptr<Queue> &queue);
  /**
   * Get the attached Queue.
   */
  Ptr<Queue> GetQueue (void) const; 
  /**
   * Attach a receive ErrorModel to the EthernetNetDevice.
   *
   * The EthernetNetDevice may optionally include an ErrorModel in
   * the packet receive chain to simulate data errors in during transmission.
   *
   * @param em the ErrorModel 
   */
  void SetReceiveErrorModel (const Ptr<ErrorModel> &em);
  /**
   * Set the encapsulation mode of this device.
   *
   * @param mode The encapsulation mode of this device.
   */
  void SetEncapsulationMode (CsmaNetDevice::EncapsulationMode mode);
  /**
   * Get the encapsulation mode of this device.
   *
   * @return The encapsulation mode of this device.
   */
  CsmaNetDevice::EncapsulationMode GetEncapsulationMode (void) const;
  /**
   * Get Tx device
   *
   * @return net device which is used for transmitting
   */
  Ptr<CsmaNetDevice> GetTxDevice (void) const;
  /**
   * Get Rx device
   *
   * @return net device which is used for receiving
   */
  Ptr<CsmaNetDevice> GetRxDevice (void) const;

  // The following methods are inherited from NetDevice base class.
  virtual void SetIfIndex (const uint32_t index);
  virtual uint32_t GetIfIndex (void) const;
  virtual Ptr<Channel> GetChannel (void) const;
  virtual bool SetMtu (const uint16_t mtu);
  virtual uint16_t GetMtu (void) const;
  virtual void SetAddress (Address address);
  virtual Address GetAddress (void) const;
  virtual bool IsLinkUp (void) const;
  virtual void AddLinkChangeCallback (Callback<void> callback);
  virtual bool IsBroadcast (void) const;
  virtual Address GetBroadcast (void) const;
  virtual bool IsMulticast (void) const;
  virtual Address GetMulticast (Ipv4Address multicastGroup) const;
  virtual Address GetMulticast (Ipv6Address addr) const;  
  virtual void SetReceiveCallback (NetDevice::ReceiveCallback cb);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  virtual bool SupportsSendFrom (void) const;
  virtual bool Send (Ptr<Packet> packet, const Address& dest, 
                        uint16_t protocolNumber);
  virtual bool SendFrom (Ptr<Packet> packet, const Address& source, const Address& dest, 
                         uint16_t protocolNumber);
  virtual Ptr<Node> GetNode (void) const;
  virtual void SetNode (Ptr<Node> node);  
  virtual bool IsPointToPoint (void) const;
  virtual bool IsBridge (void) const;
  virtual bool NeedsArp (void) const;

protected:
  virtual void DoDispose (void);

private:
  static const uint16_t DEFAULT_MTU = 1500;

  EthernetNetDevice &operator = (const EthernetNetDevice &o);
  EthernetNetDevice (const EthernetNetDevice &o);

  void NotifyLinkUp (void);
  bool NonPromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                   const Address &from);
  bool PromiscReceiveFromDevice (Ptr<NetDevice> device, Ptr<const Packet> packet, uint16_t protocol,
                                const Address &from, const Address &to, NetDevice::PacketType packetType);
                                  
  bool m_linkUp;
  CsmaNetDevice::EncapsulationMode m_encapMode;
  Ptr<Node> m_node;
  Ptr<EthernetChannel> m_channel;
  Ptr<CsmaNetDevice> m_txDev;
  Ptr<CsmaNetDevice> m_rxDev;
  uint32_t m_deviceId;
  uint32_t m_ifIndex;
  uint32_t m_mtu;
  Mac48Address m_address;

  ProxyTracedCallback m_macTxTrace;
  ProxyTracedCallback m_macTxDropTrace;
  ProxyTracedCallback m_macPromiscRxTrace;
  ProxyTracedCallback m_macRxTrace;
  ProxyTracedCallback m_phyTxBeginTrace;
  ProxyTracedCallback m_phyTxEndTrace;
  ProxyTracedCallback m_phyTxDropTrace;
  ProxyTracedCallback m_phyRxEndTrace;
  ProxyTracedCallback m_phyRxDropTrace;
  ProxyTracedCallback m_promiscSnifferTrace;
  
  TracedCallback<> m_linkChangeCallbacks;

  NetDevice::ReceiveCallback m_rxCallback;
  NetDevice::PromiscReceiveCallback m_promiscRxCallback;  
};

}; // namespace ns3

#endif // ETHERNET_NET_DEVICE_H

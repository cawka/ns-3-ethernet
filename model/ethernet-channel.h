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

#ifndef ETHERNET_CHANNEL_H
#define ETHERNET_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/ptr.h"
#include "ns3/nstime.h"
#include "ns3/data-rate.h"
#include "ns3/csma-channel.h"
#include "ethernet-net-device.h"

namespace ns3 {

class EthernetNetDevice;
class CsmaChannel;

/**
 * \brief Ethernet Channel.
 */
class EthernetChannel : public Channel
{
public:
  static TypeId GetTypeId (void);
  /**
   * @brief Create a EthernetChannel
   */
  EthernetChannel ();
  virtual ~EthernetChannel ();  
  /**
   * @brief Attach a given EthernetNetDevice to this channel
   * @param device pointer to the EthernetNetDevice to attach to the channel
   */
  void Attach (const Ptr<EthernetNetDevice> &dev);
  /**
   * @return Returns the total number of devices including devices
   * that have been detached from the channel.
   */
  virtual uint32_t GetNDevices (void) const;
  /**
   * @return Get a NetDevice pointer to a connected network device.
   *
   * @param i The index of the net device.
   * @return Returns the pointer to the net device that is associated
   * with deviceId i.
   */
  virtual Ptr<NetDevice> GetDevice (uint32_t i) const;
  /**
   * @brief Get EthernetNetDevice corresponding to index i on this channel
   * @param i Index number of the device requested
   * @returns Ptr to EthernetNetDevice requested
   */
  Ptr<EthernetNetDevice> GetEthernetDevice (uint32_t i) const;
  /**
   * Assign data rate to the channel
   */
  bool SetDataRate (DataRate bps);
  /**
   * Get the assigned data rate of the channel
   *
   * @return the DataRate to be used by device transmitters.
   */
  DataRate GetDataRate (void) const;
  /**
   * Assign speed-of-light delay delay to the channel
   */
  bool SetDelay (Time delay);    
  /**
   * Get the assigned speed-of-light delay of the channel
   *
   * @return Returns the delay used by the channel.
   */
  Time GetDelay (void) const;
  
protected:
  void DoDispose ();

private:
  static const uint32_t N_DEVICES = 2;
  
  Ptr<EthernetNetDevice> m_devices[N_DEVICES];
  uint32_t m_nDevices;
  
  Ptr<CsmaChannel> m_chan0;
  Ptr<CsmaChannel> m_chan1;

  DataRate      m_bps;
  Time          m_delay;
};

} // namespace ns3

#endif /* ETHERNET_CHANNEL_H */

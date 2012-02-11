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


#include "ethernet-channel.h"
#include "ns3/assert.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("EthernetChannel");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (EthernetChannel);

TypeId 
EthernetChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::EthernetChannel")
    .SetParent<Channel> ()
    .AddConstructor<EthernetChannel> ()
    .AddAttribute ("DataRate", 
                   "The transmission data rate to be provided to devices connected to the channel",
                   DataRateValue (DataRate (0xffffffff)),
                   MakeDataRateAccessor (&EthernetChannel::SetDataRate,
                                         &EthernetChannel::GetDataRate),
                   MakeDataRateChecker ())
    .AddAttribute ("Delay", "Transmission delay through the channel",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&EthernetChannel::SetDelay,
                                     &EthernetChannel::GetDelay),
                   MakeTimeChecker ())
    ;
  return tid;
}

EthernetChannel::EthernetChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();

  m_chan0 = CreateObject<CsmaChannel> ();
  m_chan1 = CreateObject<CsmaChannel> ();
}

EthernetChannel::~EthernetChannel ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

void
EthernetChannel::DoDispose ()
{
  NS_LOG_FUNCTION_NOARGS ();
  
  m_chan0 = 0;
  m_chan1 = 0;

  Channel::DoDispose ();
}

void
EthernetChannel::Attach (const Ptr<EthernetNetDevice> &dev)
{
  NS_LOG_FUNCTION (this << dev);

  NS_ASSERT_MSG(m_nDevices < N_DEVICES, "Only two devices permitted");
  NS_ASSERT(dev != 0);

  m_devices[m_nDevices++] = dev;

  if (m_nDevices == N_DEVICES)
    {
      m_devices[0]->GetTxDevice ()->Attach (m_chan0);
      m_devices[0]->GetRxDevice ()->Attach (m_chan1);
      m_devices[1]->GetTxDevice ()->Attach (m_chan1);
      m_devices[1]->GetRxDevice ()->Attach (m_chan0);
    }
}

uint32_t 
EthernetChannel::GetNDevices (void) const
{
  return m_nDevices;
}

Ptr<NetDevice>
EthernetChannel::GetDevice (uint32_t i) const
{
  return GetEthernetDevice (i);
}

Ptr<EthernetNetDevice>
EthernetChannel::GetEthernetDevice (uint32_t i) const
{
  NS_ASSERT(i < 2);
  return m_devices[i];
}

DataRate
EthernetChannel::GetDataRate (void) const
{
  return m_bps;
}

bool
EthernetChannel::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this << bps);
  m_bps = bps;
  DataRateValue bpsValue (bps);
  m_chan0->SetAttribute ("DataRate", bpsValue);
  m_chan1->SetAttribute ("DataRate", bpsValue);  
  return true;
}

Time
EthernetChannel::GetDelay (void) const
{
  return m_delay;
}

bool
EthernetChannel::SetDelay (Time delay)
{
  NS_LOG_FUNCTION (this << delay);
  m_delay = delay;
  TimeValue delayValue (delay);
  m_chan0->SetAttribute ("Delay", delayValue);
  m_chan1->SetAttribute ("Delay", delayValue);
  return true;  
}


} // namespace ns3

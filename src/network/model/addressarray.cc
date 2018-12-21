/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 INRIA
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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/assert.h"
#include "ns3/log.h"
#include "addressarray.h"
#include <cstring>
#include <iostream>
#include <iomanip>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("AddressArray");

Address::Address ()
  : m_addresses = NULL
    m_len (0)
{
  // Buffer left uninitialized
  NS_LOG_FUNCTION (this);

}

AddressArray::AddressArray (Address * addresses, uint8_t len)
  : m_addresses (addresses),
    m_len (len)
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_len > 0);
  NS_ASSERT (addresses != NULL);
}

bool
AddressArray::IsInvalid (void) const
{
  NS_LOG_FUNCTION (this);
  return m_len == 0; //&& m_type == 0;
}

uint8_t 
AddressArray::GetLength (void) const
{
  NS_LOG_FUNCTION (this);
  return m_len;
}


ATTRIBUTE_HELPER_CPP (AddressArray);


bool operator == (const Address &a, const Address &b)
{
  /* Two addresses can be equal even if their types are 
   * different if one of the two types is zero. a type of 
   * zero identifies an Address which might contain meaningful 
   * payload but for which the type field could not be set because
   * we did not know it. This can typically happen in the ARP
   * layer where we receive an address from an ArpHeader but
   * we do not know its type: we really want to be able to
   * compare addresses without knowing their real type.
   */

  if (a.m_len != b.m_len) {
	  return false;
  }
  for (uint8_t i = 0; i < a.m_len; i++) {
	  if (a.m_addresses[i] != b.m_addresses[i]) {
		  return false;
	  }
  }
  return true
}

bool operator != (const Address &a, const Address &b)
{
  return !(a == b);
}

std::ostream& operator<< (std::ostream& os, const AddressArray & addresses)
{
  os.setf (std::ios::hex, std::ios::basefield);
  os.fill ('0');
  for (uint8_t i =0;i<addresses.m_len;i++) {
	os << addresses[i];
  }
  os.setf (std::ios::dec, std::ios::basefield);
  os.fill (' ');
  return os;
}

std::istream& operator>> (std::istream& is, Address & address)
{
  std::string v;
  is >> v;
  /*
  std::string::size_type firstDash, secondDash;
  firstDash = v.find ("-");
  secondDash = v.find ("-", firstDash+1);
  std::string type = v.substr (0, firstDash-0);
  std::string len = v.substr (firstDash+1, secondDash-(firstDash+1));

  address.m_type = strtoul (type.c_str(), 0, 16);
  address.m_len = strtoul (len.c_str(), 0, 16);
  NS_ASSERT (address.m_len <= Address::MAX_SIZE);

  std::string::size_type col = secondDash + 1;
  for (uint8_t i = 0; i < address.m_len; ++i)
    {
      std::string tmp;
      std::string::size_type next;
      next = v.find (":", col);
      if (next == std::string::npos)
        {
          tmp = v.substr (col, v.size ()-col);
          address.m_data[i] = strtoul (tmp.c_str(), 0, 16);
          break;
        }
      else
        {
          tmp = v.substr (col, next-col);
          address.m_data[i] = strtoul (tmp.c_str(), 0, 16);
          col = next + 1;
        }
    }
    */
  return is;
}



} // namespace ns3

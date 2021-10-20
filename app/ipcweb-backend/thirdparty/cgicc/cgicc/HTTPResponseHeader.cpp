/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTTPResponseHeader.cpp,v 1.9 2014/04/23 20:55:09 sebdiaz Exp $
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth <sbooth@gnu.org>
 *                       2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
 *  Part of the GNU cgicc library, http://www.gnu.org/software/cgicc
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA 
 */

#ifdef __GNUG__
#  pragma implementation
#endif

#include "HTTPResponseHeader.h"

// ============================================================
// Class HTTPResponseHeader
// ============================================================
cgicc::HTTPResponseHeader::HTTPResponseHeader(const std::string& version,
					      int status_code,
					      const std::string& reason)
  : MStreamable(),
    fHTTPVersion(version),
    fStatusCode(status_code),
    fReasonPhrase(reason)
{
  fHeaders.reserve(5);
}

cgicc::HTTPResponseHeader::~HTTPResponseHeader()
{}

cgicc::HTTPResponseHeader&
cgicc::HTTPResponseHeader::addHeader(const std::string& header)
{
  fHeaders.push_back(header); 
  return *this; 
}

cgicc::HTTPResponseHeader&
cgicc::HTTPResponseHeader::addHeader(const std::string& name,
				     const std::string& value)
{ 
  fHeaders.push_back(name + ": " + value); 
  return *this; 
}

cgicc::HTTPResponseHeader&
cgicc::HTTPResponseHeader::setCookie(const HTTPCookie& cookie)
{ 
  fCookies.push_back(cookie); 
  return *this; 
}

void 
cgicc::HTTPResponseHeader::render(std::ostream& out)	const
{
  std::vector<std::string>::const_iterator iter;
  std::vector<HTTPCookie>::const_iterator cookie_iter; 
  
  out << fHTTPVersion << ' ' << fStatusCode << ' ' << fReasonPhrase 
      << std::endl;
  
  for(iter = fHeaders.begin(); iter != fHeaders.end(); ++iter)
    out << *iter << std::endl;
  
  for(cookie_iter = getCookies().begin(); 
      cookie_iter != getCookies().end(); 
      ++cookie_iter)
    out << *cookie_iter << std::endl;
  
  out << std::endl;
}

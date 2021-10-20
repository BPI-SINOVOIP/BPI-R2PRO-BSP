/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTTPRedirectHeader.cpp,v 1.10 2014/04/23 20:55:08 sebdiaz Exp $
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

#include "HTTPRedirectHeader.h"

// ============================================================
// Class HTTPRedirectHeader
// ============================================================
cgicc::HTTPRedirectHeader::HTTPRedirectHeader(const std::string& url) 
    : HTTPHeader(url) , fStatus(-1)
 {}
 
cgicc::HTTPRedirectHeader::HTTPRedirectHeader(const std::string& url,bool permanent) 
  : HTTPHeader(url)
{
  fStatus = permanent ? 301 : 302;
}

cgicc::HTTPRedirectHeader::~HTTPRedirectHeader()
{}

void 
cgicc::HTTPRedirectHeader::render(std::ostream& out) 	const
{
  if(fStatus == 301)
    out << "Status: 301 Moved Permanently" << std::endl;
  else if(fStatus == 302)
    out << "Status: 302 Found" << std::endl;
  out << "Location: " << getData() << std::endl;
  
  if(false == getCookies().empty()) {
    std::vector<HTTPCookie>::const_iterator iter; 
    
    for(iter = getCookies().begin(); iter != getCookies().end(); ++iter)
      out << *iter << std::endl;
  }
  
  out << std::endl;
}

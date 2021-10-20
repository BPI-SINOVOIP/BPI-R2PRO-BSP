/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTTPContentHeader.cpp,v 1.8 2014/04/23 20:55:06 sebdiaz Exp $
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

#include "HTTPContentHeader.h"

// ============================================================
// Class HTTPContentHeader
// ============================================================
cgicc::HTTPContentHeader::HTTPContentHeader(const std::string& mimeType) 
  : HTTPHeader(mimeType)
{}

cgicc::HTTPContentHeader::~HTTPContentHeader()
{}

void 
cgicc::HTTPContentHeader::render(std::ostream& out)	const
{
  out << "Content-Type: " << getData() << std::endl;
  
  std::vector<HTTPCookie>::const_iterator iter; 
  for(iter = getCookies().begin(); iter != getCookies().end(); ++iter)
    out << *iter << std::endl;
  
  out << std::endl;
}

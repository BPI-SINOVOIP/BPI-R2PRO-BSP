/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTTPCookie.cpp,v 1.12 2014/04/23 20:55:07 sebdiaz Exp $
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

#include "HTTPCookie.h"
#include "CgiUtils.h"

// ============================================================
// Class HTTPCookie
// ============================================================
cgicc::HTTPCookie::HTTPCookie()
  : fMaxAge(0),
    fSecure(false),
    fRemoved(false)
{}

cgicc::HTTPCookie::HTTPCookie(const std::string& name, 
			      const std::string& value)
  : fName(name),
    fValue(value),
    fMaxAge(0),
    fSecure(false),
    fRemoved(false)
{}

cgicc::HTTPCookie::HTTPCookie(const std::string& name, 
			      const std::string& value, 
			      const std::string& comment, 
			      const std::string& domain, 
			      unsigned long maxAge, 
			      const std::string& path,
			      bool secure)
  : fName(name),
    fValue(value), 
    fComment(comment), 
    fDomain(domain), 
    fMaxAge(maxAge),
    fPath(path), 
    fSecure(secure),
    fRemoved(false)
{}

cgicc::HTTPCookie::HTTPCookie(const HTTPCookie& cookie)
  : MStreamable(),
    fName(cookie.fName), 
    fValue(cookie.fValue), 
    fComment(cookie.fComment),
    fDomain(cookie.fDomain), 
    fMaxAge(cookie.fMaxAge),
    fPath(cookie.fPath), 
    fSecure(cookie.fSecure),
    fRemoved(cookie.fRemoved)
{}

cgicc::HTTPCookie::~HTTPCookie()
{}

bool 
cgicc::HTTPCookie::operator== (const HTTPCookie& cookie) const
{
  return (stringsAreEqual(fName, cookie.fName)
	  && stringsAreEqual(fValue, cookie.fValue)
	  && stringsAreEqual(fComment, cookie.fComment)
	  && stringsAreEqual(fDomain, cookie.fDomain)
	  && fMaxAge == cookie.fMaxAge
	  && stringsAreEqual(fPath, cookie.fPath)
	  && fSecure == cookie.fSecure
	  && fRemoved == cookie.fRemoved);
}

void 
cgicc::HTTPCookie::render(std::ostream& out) 	const
{
  out << "Set-Cookie:" << fName << '=' << fValue;
  if(false == fComment.empty())
    out << "; Comment=" << fComment;
  if(false == fDomain.empty())
    out << "; Domain=" << fDomain;
  if(fRemoved)
    out << "; Expires=Fri, 01-Jan-1971 01:00:00 GMT;";
  else if(0 != fMaxAge)
      out << "; Max-Age=" << fMaxAge;
  if(false == fPath.empty())
    out << "; Path=" << fPath;
  if(true == fSecure)
    out << "; Secure";
  
  out << "; Version=1";
}

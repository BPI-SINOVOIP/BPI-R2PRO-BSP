/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTMLAttributeList.cpp,v 1.8 2014/04/23 20:55:05 sebdiaz Exp $
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

#include "HTMLAttributeList.h"
#include "CgiUtils.h"

// ============================================================
// Class HTMLAttributeList
// ============================================================
cgicc::HTMLAttributeList::HTMLAttributeList()
{
  fAttributes.reserve(5);
}

cgicc::HTMLAttributeList::HTMLAttributeList(const HTMLAttribute& head) 
{
  fAttributes.reserve(5);
  fAttributes.push_back(head);
}

cgicc::HTMLAttributeList::HTMLAttributeList(const HTMLAttributeList& list)
{
  this->operator=(list);
}

cgicc::HTMLAttributeList::~HTMLAttributeList()
{}

cgicc::HTMLAttributeList&
cgicc::HTMLAttributeList::operator= (const HTMLAttributeList& list)
{
  fAttributes = list.fAttributes;
  return *this;
}

cgicc::HTMLAttributeList&
cgicc::HTMLAttributeList::set(const std::string& name)
{ 
  fAttributes.push_back(HTMLAttribute(name));
  return *this;
}

cgicc::HTMLAttributeList&
cgicc::HTMLAttributeList::set(const std::string& name, 
			      const std::string& value)
{ 
  fAttributes.push_back(HTMLAttribute(name, value));
  return *this;
}

void 
cgicc::HTMLAttributeList::render(std::ostream& out) 	const
{
  std::vector<HTMLAttribute>::const_iterator iter;
  for(iter = fAttributes.begin(); iter != fAttributes.end(); ++iter) {
    out << *iter << ' ';
  }
}

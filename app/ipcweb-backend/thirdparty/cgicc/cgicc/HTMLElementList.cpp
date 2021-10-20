/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTMLElementList.cpp,v 1.8 2014/04/23 20:55:06 sebdiaz Exp $
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

#include <new>

#include "HTMLElementList.h"


// ============================================================
// Class HTMLElementList
// ============================================================
cgicc::HTMLElementList::HTMLElementList()
{
  fElements.reserve(5);
}

cgicc::HTMLElementList::HTMLElementList(const HTMLElement& head) 
{
  fElements.reserve(5);
  fElements.push_back(head.clone());
}

cgicc::HTMLElementList::HTMLElementList(const HTMLElementList& list)
{
  this->operator=(list);
}

cgicc::HTMLElementList::~HTMLElementList()
{
  std::vector<HTMLElement*>::const_iterator iter;
  for(iter = fElements.begin(); iter != fElements.end(); ++iter)
    delete *iter;
}

cgicc::HTMLElementList&
cgicc::HTMLElementList::operator= (const HTMLElementList& list)
{
  fElements = list.fElements;
  
  std::vector<HTMLElement*>::iterator iter;
  for(iter = fElements.begin(); iter != fElements.end(); ++iter)
    *iter = (*iter)->clone();
  
  return *this;
}

cgicc::HTMLElementList&
cgicc::HTMLElementList::add(const HTMLElement& element)
{ 
  fElements.push_back(element.clone());
  return *this;
}

cgicc::HTMLElementList&
cgicc::HTMLElementList::add(HTMLElement *element)
{ 
  fElements.push_back(element);
  return *this;
}

void 
cgicc::HTMLElementList::render(std::ostream& out) 	const
{
  std::vector<HTMLElement*>::const_iterator iter;
  for(iter = fElements.begin(); iter != fElements.end(); ++iter)
    (*iter)->render(out);
}

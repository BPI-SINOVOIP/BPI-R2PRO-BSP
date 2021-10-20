/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: HTMLElement.cpp,v 1.10 2014/04/23 20:55:05 sebdiaz Exp $
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
#include <cstring>
#include "HTMLElement.h"
#include "HTMLElementList.h"

// ============================================================
// Class HTMLElement
// ============================================================
cgicc::HTMLElement::HTMLElement(const HTMLElement& element)
  : MStreamable(),
    fAttributes(0),
    fEmbedded(0)
{
  this->operator= (element);
}

cgicc::HTMLElement::HTMLElement(const HTMLAttributeList *attributes,
				const HTMLElement *embedded,
				const std::string *data,
				EElementType type)
  : fAttributes(0),
    fEmbedded(0),
    fData(),
    fType(type),
    fDataSpecified(false)
{
  if(0 != attributes)
    fAttributes = new HTMLAttributeList(*attributes);

  if(0 != embedded)
    fEmbedded = new HTMLElementList(*embedded);

  if(0 != data) {
    fData = *data;
    fDataSpecified = true;
  }
}

cgicc::HTMLElement::~HTMLElement()
{
  delete fAttributes;
  delete fEmbedded;
}

bool
cgicc::HTMLElement::operator== (const HTMLElement& element) const
{
  // this is really lame, but only necessary for template instantiation
  return (strcmp(getName(), element.getName()) == 0
	  && fDataSpecified == element.fDataSpecified);
}

cgicc::HTMLElement&
cgicc::HTMLElement::operator= (const HTMLElement& element)
{
  // avoid memory leak; bug fix from Francois Degros
  delete fAttributes;
  delete fEmbedded;

  fAttributes    = element.fAttributes;
  fEmbedded      = element.fEmbedded;
  fData          = element.fData;
  fType          = element.fType;
  fDataSpecified = element.fDataSpecified;

  // perform a deep copy
  if(0 != fAttributes)
    fAttributes = new HTMLAttributeList(*fAttributes);
  
  if(0 != fEmbedded)
    fEmbedded = new HTMLElementList(*fEmbedded);
  
  return *this;
}

void 
cgicc::HTMLElement::setAttributes(const HTMLAttributeList& attributes)
{ 
  delete fAttributes;
  fAttributes = new HTMLAttributeList(attributes);
}

void
cgicc::HTMLElement::setEmbedded(const HTMLElementList& embedded)
{
  delete fEmbedded;
  fEmbedded = new HTMLElementList(embedded);
}

cgicc::HTMLElement&
cgicc::HTMLElement::add(const HTMLElement& element)
{
  if(0 == fEmbedded)
    fEmbedded = new HTMLElementList();
  fEmbedded->add(element);
  return *this;
}

cgicc::HTMLElement&
cgicc::HTMLElement::add(HTMLElement *element)
{
  if(0 == fEmbedded)
    fEmbedded = new HTMLElementList();
  fEmbedded->add(element);
  return *this;
}

cgicc::HTMLElement&
cgicc::HTMLElement::set(const std::string& name)
{
  if(0 == fAttributes)
    fAttributes = new HTMLAttributeList();
  fAttributes->set(name);
  return *this;
}

cgicc:: HTMLElement&
cgicc::HTMLElement::set(const std::string& name,
			const std::string& value)
{
  if(0 == fAttributes)
    fAttributes = new HTMLAttributeList();
  fAttributes->set(name, value);
  return *this;
}

void
cgicc::HTMLElement::render(std::ostream& out) 	const
{
  if(eBoolean == getType() && false == dataSpecified()) {
    /* no embedded elements */
    if(0 == fEmbedded) {
      swapState();
      /* getState() == true ===> element is active */
      if(true == getState()) {
	out << '<' << getName();
	/* render attributes, if present */
	if(0 != fAttributes) {
	  out << ' ';
	  fAttributes->render(out);
	}
	out << '>';
      }
      else
	out << "</" << getName() << '>';
    }
    /* embedded elements present */
    else {
      out << '<' << getName();
      /* render attributes, if present */
      if(0 != fAttributes) {
	out << ' ';
	fAttributes->render(out);
      }
      out << '>';
      fEmbedded->render(out);
      out << "</" << getName() << '>';
    }
  }
  /* For non-boolean elements */
  else {
    if(eAtomic == getType()) {
      out << '<' << getName();
      if(0 != fAttributes) {
	out << ' ';
	fAttributes->render(out);
      }
      out << " />";
    }
    else {
      out << '<' << getName();
      if(0 != fAttributes) {
	out << ' ';
	fAttributes->render(out);
      }
      out << '>';
      
      if(0 != fEmbedded)
	fEmbedded->render(out);
      else
	out << getData();
      out << "</" << getName() << '>';
    }
  }
}

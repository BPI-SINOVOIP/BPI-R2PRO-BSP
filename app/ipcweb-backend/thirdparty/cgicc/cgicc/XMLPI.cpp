/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: XMLPI.cpp,v 1.1 2008/01/19 15:43:57 sebdiaz Exp $
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth <sbooth@gnu.org>
 *                       2007 David Roberts
		     2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
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
#include "XMLPI.h"


cgicc::XMLPI::XMLPI(std::string name) : MStreamable(), fAttributes(0), fName(name) {}

cgicc::XMLPI::~XMLPI() {
	delete fAttributes;
}

cgicc::XMLPI& cgicc::XMLPI::operator= (const XMLPI& element) {
	delete fAttributes;
	fAttributes = element.fAttributes;
	if(fAttributes != 0) fAttributes = new HTMLAttributeList(*fAttributes);
	return *this;
}

void cgicc::XMLPI::setAttributes(const HTMLAttributeList& attributes) {
	delete fAttributes;
	fAttributes = new HTMLAttributeList(attributes);
}

cgicc::XMLPI& cgicc::XMLPI::set(const std::string& name) {
	if(fAttributes == 0) fAttributes = new HTMLAttributeList();
	fAttributes->set(name);
	return *this;
}

cgicc::XMLPI& cgicc::XMLPI::set(const std::string& name, const std::string& value) {
	if(fAttributes == 0) fAttributes = new HTMLAttributeList();
	fAttributes->set(name, value);
	return *this;
}

void cgicc::XMLPI::render(std::ostream& out) const {
	out << "<?" << fName;
	if(getAttributes() != 0) {
		out << ' ';
		fAttributes->render(out);
	}
	out << "?>";
}

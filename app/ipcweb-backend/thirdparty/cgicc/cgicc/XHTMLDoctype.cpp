/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: XHTMLDoctype.cpp,v 1.2 2014/12/07 14:33:03 sebdiaz Exp $
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
#include "XHTMLDoctype.h"


cgicc::XHTMLDoctype::XHTMLDoctype(EDocumentType type) : fType(type) {}

cgicc::XHTMLDoctype::~XHTMLDoctype() {}

void cgicc::XHTMLDoctype::render(std::ostream &out) const {
	out << "<!DOCTYPE html";
	bool bHTML5 = false;

	switch(fType) {
		case eStrict: out << " PUBLIC \"-//W3C//DTD XHTML 1.0  Strict"; break;
		case eTransitional: out << " PUBLIC \"-//W3C//DTD XHTML 1.0  Transitional"; break;
		case eFrames: out << " PUBLIC \"-//W3C//DTD XHTML 1.0  Frameset"; break;
		case eHTML5: bHTML5 = true; break;
	}
	
	if(bHTML5 == false)
		out << "//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-";
	
	switch(fType) {
		case eStrict: out << "strict"; break;
		case eTransitional: out << "transitional"; break;
		case eFrames: out << "frameset"; break;
		case eHTML5: break;
	}
	out << ".dtd\">";
}


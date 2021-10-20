/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: CgiInput.cpp,v 1.9 2014/04/23 20:55:03 sebdiaz Exp $
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110, USAa
 */

#ifdef __GNUG__
#  pragma implementation
#endif

#include <iostream>
#include <cstdlib>

#include "CgiInput.h"

// ========== Destructor

cgicc::CgiInput::~CgiInput()
{}

// ========== Members

size_t 
cgicc::CgiInput::read(char *data, 
		      size_t length)
{
  std::cin.read(data, length);
  return std::cin.gcount();
}

std::string
cgicc::CgiInput::getenv(const char *varName)
{
  char *var = std::getenv(varName);
  return (0 == var) ? std::string("") : var;
}

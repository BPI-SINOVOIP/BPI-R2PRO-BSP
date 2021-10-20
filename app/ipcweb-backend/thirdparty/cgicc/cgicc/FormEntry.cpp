/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: FormEntry.cpp,v 1.14 2014/04/23 20:55:04 sebdiaz Exp $
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
#include <cstdlib>

#include "FormEntry.h"

// local macro for integer maximum
#define MAX(a, b) ((a) > (b) ? (a) : (b))

cgicc::FormEntry& 
cgicc::FormEntry::operator= (const FormEntry& entry)
{
  fName  = entry.fName;
  fValue = entry.fValue;

  return *this;
}

long
cgicc::FormEntry::getIntegerValue(long min, 
				  long max) 		const
{
  long value = std::atol(fValue.c_str());

  if(value > max)
    value = max;
  else if(value < min)
    value = min;
  
  return value;
}

long
cgicc::FormEntry::getIntegerValue(long min, 
				  long max,
				  bool& bounded) 	const
{
  long value = std::atol(fValue.c_str());

  bounded = false;

  if(value > max) {
    value = max;
    bounded = true;
  }
  else if(value < min) {
    value = min;
    bounded = true;
  }
  
  return value;
}

double
cgicc::FormEntry::getDoubleValue(double min, 
				 double max) 		const
{
  double value = std::atof(fValue.c_str());
  if(value > max)
    value = max;
  else if(value < min)
    value = min;
  
  return value;
}

double
cgicc::FormEntry::getDoubleValue(double min, 
				 double max,
				 bool& bounded) 	const
{
  double value = std::atof(fValue.c_str());

  bounded = false;

  if(value > max) {
    value = max;
    bounded = true;
  }
  else if(value < min) {
    value = min;
    bounded = true;
  }
  
  return value;
}

std::string
cgicc::FormEntry::makeString(std::string::size_type maxLen, 
			     bool allowNewlines)	const
{
  std::string::size_type	len 		= 0;
  std::string::size_type	avail 		= maxLen;
  std::string::size_type	crCount 	= 0;
  std::string::size_type	lfCount 	= 0;	
  std::string::const_iterator 	src 		= fValue.begin();
  std::string::const_iterator 	lim 		= fValue.end();
  std::string 			dst;


  while(src != lim && len < avail) {

    // handle newlines
    if('\r' == *src || '\n' == *src) {
      crCount = 0;
      lfCount = 0;
      
      // Count the number of each kind of line break ('\r' and '\n')
      while( ('\r' == *src || '\n' == *src) && (src != lim)) {
	if('\r' == *src) 
	  crCount++;
	else 
	  lfCount++;
	++src;
      }
      
      // if newlines are allowed, add them
      if(allowNewlines) {
	// output the larger value
	int lfsAdd = MAX(crCount, lfCount);
	dst.append(lfsAdd, '\n');
	len += lfsAdd;		
      }
    }
    // just append all other characters
    else {
      dst.append(1, *src);
      ++len;
      ++src;
    }
  }
  
  return dst;
}

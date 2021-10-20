/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: CgiUtils.cpp,v 1.20 2014/04/23 20:55:03 sebdiaz Exp $
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

#include <stdexcept>
#include <memory>
#include <vector>
#include <iterator> 	// for distance
#include <cctype> 	// for toupper, isxdigit

#include "CgiUtils.h"

// case-insensitive string comparison
// This code based on code from 
// "The C++ Programming Language, Third Edition" by Bjarne Stroustrup
bool 
cgicc::stringsAreEqual(const std::string& s1, 
		       const std::string& s2)
{
  std::string::const_iterator p1 = s1.begin();
  std::string::const_iterator p2 = s2.begin();
  std::string::const_iterator l1 = s1.end();
  std::string::const_iterator l2 = s2.end();

  while(p1 != l1 && p2 != l2) {
    if(std::toupper(*(p1++)) != std::toupper(*(p2++)))
      return false;
  }

  return (s2.size() == s1.size()) ? true : false;
}

// case-insensitive string comparison
bool 
cgicc::stringsAreEqual(const std::string& s1, 
		       const std::string& s2,
		       size_t n)
{
  std::string::const_iterator p1 = s1.begin();
  std::string::const_iterator p2 = s2.begin();
  bool good = (n <= s1.length() && n <= s2.length());
  std::string::const_iterator l1 = good ? (s1.begin() + n) : s1.end();
  std::string::const_iterator l2 = good ? (s2.begin() + n) : s2.end();

  while(p1 != l1 && p2 != l2) {
    if(std::toupper(*(p1++)) != std::toupper(*(p2++)))
      return false;
  }
  
  return good;
}

std::string
cgicc::charToHex(char c)
{
  std::string result;
  char first, second;

  first = (c & 0xF0) / 16;
  first += first > 9 ? 'A' - 10 : '0';
  second = c & 0x0F;
  second += second > 9 ? 'A' - 10 : '0';

  result.append(1, first);
  result.append(1, second);
  
  return result;
}

char
cgicc::hexToChar(char first,
		 char second)
{
  int digit;
  
  digit = (first >= 'A' ? ((first & 0xDF) - 'A') + 10 : (first - '0'));
  digit *= 16;
  digit += (second >= 'A' ? ((second & 0xDF) - 'A') + 10 : (second - '0'));
  return static_cast<char>(digit);
}

/* 
   From the HTML standard: 
   <http://www.w3.org/TR/html4/interact/forms.html#h-17.13.4.1>

   application/x-www-form-urlencoded  

   This is the default content type. Forms submitted with this content
   type must be encoded as follows:

   1. Control names and values are escaped. Space characters are
   replaced by `+', and then reserved characters are escaped as
   described in [RFC1738], section 2.2: Non-alphanumeric characters
   are replaced by `%HH', a percent sign and two hexadecimal digits
   representing the ASCII code of the character. Line breaks are
   represented as "CR LF" pairs (i.e., `%0D%0A').  
   2. The control names/values are listed in the order they appear in
   the document. The name is separated from the value by `=' and
   name/value pairs are separated from each other by `&'.


   Note RFC 1738 is obsoleted by RFC 2396.  Basically it says to
   escape out the reserved characters in the standard %xx format.  It
   also says this about the query string:
   
   query         = *uric
   uric          = reserved | unreserved | escaped
   reserved      = ";" | "/" | "?" | ":" | "@" | "&" | "=" | "+" |
   "$" | ","
   unreserved    = alphanum | mark
   mark          = "-" | "_" | "." | "!" | "~" | "*" | "'" |
   "(" | ")"
   escaped = "%" hex hex */
 
std::string
cgicc::form_urlencode(const std::string& src)
{
  std::string result;
  std::string::const_iterator iter;
  
  for(iter = src.begin(); iter != src.end(); ++iter) {
    switch(*iter) {
    case ' ':
      result.append(1, '+');
      break;
      // alnum
    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G':
    case 'H': case 'I': case 'J': case 'K': case 'L': case 'M': case 'N':
    case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T': case 'U':
    case 'V': case 'W': case 'X': case 'Y': case 'Z':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
    case 'h': case 'i': case 'j': case 'k': case 'l': case 'm': case 'n':
    case 'o': case 'p': case 'q': case 'r': case 's': case 't': case 'u':
    case 'v': case 'w': case 'x': case 'y': case 'z':
    case '0': case '1': case '2': case '3': case '4': case '5': case '6':
    case '7': case '8': case '9':
      // mark
    case '-': case '_': case '.': case '!': case '~': case '*': case '\'': 
    case '(': case ')':
      result.append(1, *iter);
      break;
      // escape
    default:
      result.append(1, '%');
      result.append(charToHex(*iter));
      break;
    }
  }
  
  return result;
}

std::string
cgicc::form_urldecode(const std::string& src)
{
  std::string result;
  std::string::const_iterator iter;
  char c;

  for(iter = src.begin(); iter != src.end(); ++iter) {
    switch(*iter) {
    case '+':
      result.append(1, ' ');
      break;
    case '%':
      // Don't assume well-formed input
      if(std::distance(iter, src.end()) >= 2
	 && std::isxdigit(*(iter + 1)) && std::isxdigit(*(iter + 2))) {
	c = *++iter;
	result.append(1, hexToChar(c, *++iter));
      }
      // Just pass the % through untouched
      else {
	result.append(1, '%');
      }
      break;
    
    default:
      result.append(1, *iter);
      break;
    }
  }
  
  return result;
}

// locate data between separators, and return it
std::string
cgicc::extractBetween(const std::string& data, 
		      const std::string& separator1, 
		      const std::string& separator2)
{
  std::string result;
  std::string::size_type start, limit;
  
  start = data.find(separator1, 0);
  if(std::string::npos != start) {
    start += separator1.length();
    limit = data.find(separator2, start);
    if(std::string::npos != limit)
      result = data.substr(start, limit - start);
  }
  
  return result;
}

// write a string
void 
cgicc::writeString(std::ostream& out, 
		   const std::string& s)
{ 
  out << s.length() << ' ';
  out.write(s.data(), s.length()); 
}

// write a long
void 
cgicc::writeLong(std::ostream& out, 
		 unsigned long l)
{ 
  out << l << ' '; 
}

// read a string
std::string
cgicc::readString(std::istream& in)
{
  std::string::size_type dataSize = 0;
  
  in >> dataSize;
  in.get(); // skip ' '
  
  // Avoid allocation of a zero-length vector
  if(0 == dataSize) {
    return std::string();
  }

  // Don't use auto_ptr, but vector instead
  // Bug reported by bostjan@optonline.net / fix by alexoss@verizon.net
  std::vector<char> temp(dataSize);

  in.read(&temp[0], dataSize);
  if(static_cast<std::string::size_type>(in.gcount()) != dataSize) {
    throw std::runtime_error("I/O error");
  }

  return std::string(&temp[0], dataSize);
}

// read a long
unsigned long
cgicc::readLong(std::istream& in)
{
  unsigned long l;

  in >> l;
  in.get(); // skip ' '
  return l;
}

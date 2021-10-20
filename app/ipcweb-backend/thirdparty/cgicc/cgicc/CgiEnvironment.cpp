/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: CgiEnvironment.cpp,v 1.31 2017/06/22 20:26:35 sebdiaz Exp $
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
 *
 */

#ifdef __GNUG__
#  pragma implementation
#endif

#include <new>
#include <memory>
#include <stdexcept>
#include <cstdlib>
#include <cctype>

#ifdef WIN32
#  include <io.h>
#  include <fcntl.h>
#  include <stdio.h>
#endif

#include "CgiEnvironment.h"

// ========== Constructor/Destructor

cgicc::CgiEnvironment::CgiEnvironment(CgiInput *input)
{
  // Create a local CgiInput object for us to use
  // In the vast majority of cases, this will be used
  // For FastCGI applications it won't but the performance hit of
  // an empty inline constructor is negligible
  CgiInput local_input;

  if(0 == input)
    readEnvironmentVariables(&local_input);
  else
    readEnvironmentVariables(input);

  // On Win32, use binary read to avoid CRLF conversion
#ifdef WIN32
#  ifdef __BORLANDC__
  setmode(_fileno(stdin), O_BINARY);
#  else
  _setmode(_fileno(stdin), _O_BINARY);
#  endif
#endif
     
  if(stringsAreEqual(fRequestMethod, "post") || stringsAreEqual(fRequestMethod, "put")) {
    // Don't use auto_ptr, but vector instead
    // Bug reported by shinra@j10n.org
    std::vector<char> data(fContentLength);
    
    if(getenv("CGICC_MAX_CONTENTLENGTH")&&getContentLength()>(long unsigned int)atoi(getenv("CGICC_MAX_CONTENTLENGTH")))
    {
        throw std::runtime_error("Malformed input");
    }
    else
    // If input is 0, use the default implementation of CgiInput
	if ( getContentLength() )
	{
	// If input is 0, use the default implementation of CgiInput
		if ( input == 0 )
		{
		if ( local_input.read( &data[0], getContentLength() ) != getContentLength() )
		throw std::runtime_error("I/O error");
		}
		else
		if ( input->read( &data[0], getContentLength() ) != getContentLength() )
		throw std::runtime_error("I/O error");

		fPostData = std::string( &data[0], getContentLength() );
	} 
  }
  
  fCookies.reserve(10);
  parseCookies();
}

cgicc::CgiEnvironment::~CgiEnvironment()
{}

// Overloaded operators
bool 
cgicc::CgiEnvironment::operator== (const CgiEnvironment& env) 		const
{
  bool result;
  
  result =  fServerPort 	== env.fServerPort;
  result &= fContentLength 	== env.fContentLength;
  result &= fUsingHTTPS 	== env.fUsingHTTPS;
  result &= fServerSoftware 	== env.fServerSoftware;
  result &= fServerName 	== env.fServerName;
  result &= fGatewayInterface 	== env.fGatewayInterface;
  result &= fServerProtocol 	== env.fServerProtocol;
  result &= fRequestMethod 	== env.fRequestMethod;
  result &= fPathInfo 		== env.fPathInfo;
  result &= fPathTranslated 	== env.fPathTranslated;
  result &= fScriptName 	== env.fScriptName;
  result &= fQueryString 	== env.fQueryString;
  result &= fRemoteHost 	== env.fRemoteHost;
  result &= fRemoteAddr 	== env.fRemoteAddr;
  result &= fAuthType 		== env.fAuthType;
  result &= fRemoteUser 	== env.fRemoteUser;
  result &= fRemoteIdent 	== env.fRemoteIdent;
  result &= fContentType 	== env.fContentType;
  result &= fAccept 		== env.fAccept;
  result &= fUserAgent 		== env.fUserAgent;
  result &= fPostData 		== env.fPostData;
  result &= fRedirectRequest 	== env.fRedirectRequest;
  result &= fRedirectURL 	== env.fRedirectURL;
  result &= fRedirectStatus 	== env.fRedirectStatus;
  result &= fReferrer 		== env.fReferrer;
  result &= fCookie 		== env.fCookie;

  return result;
}

cgicc::CgiEnvironment& 
cgicc::CgiEnvironment::operator= (const CgiEnvironment& env)
{
  fServerPort 		= env.fServerPort;
  fContentLength 	= env.fContentLength;
  fUsingHTTPS 		= env.fUsingHTTPS;
  fServerSoftware 	= env.fServerSoftware;
  fServerName 		= env.fServerName;
  fGatewayInterface 	= env.fGatewayInterface;
  fServerProtocol 	= env.fServerProtocol;
  fRequestMethod 	= env.fRequestMethod;
  fPathInfo 		= env.fPathInfo;
  fPathTranslated 	= env.fPathTranslated;
  fScriptName 		= env.fScriptName;
  fQueryString 		= env.fQueryString;
  fRemoteHost 		= env.fRemoteHost;
  fRemoteAddr 		= env.fRemoteAddr;
  fAuthType 		= env.fAuthType;
  fRemoteUser 		= env.fRemoteUser;
  fRemoteIdent 		= env.fRemoteIdent;
  fContentType 		= env.fContentType;
  fAccept 		= env.fAccept;
  fUserAgent 		= env.fUserAgent;
  fPostData 		= env.fPostData;
  fRedirectRequest 	= env.fRedirectRequest;
  fRedirectURL 		= env.fRedirectURL;
  fRedirectStatus 	= env.fRedirectStatus;
  fReferrer 		= env.fReferrer;
  fCookie 		= env.fCookie;

  fCookies.clear();
  fCookies.reserve(env.fCookies.size());
  parseCookies();

  return *this;
}

void
cgicc::CgiEnvironment::parseCookies()
{
  std::string data = fCookie;

  if(false == data.empty()) {
    std::string::size_type pos;
    std::string::size_type oldPos = 0;

    while(true) {
      // find the ';' terminating a name=value pair
      pos = data.find(";", oldPos);

      // if no ';' was found, the rest of the string is a single cookie
      if(std::string::npos == pos) {
	parseCookie(data.substr(oldPos));
	return;
      }

      // otherwise, the string contains multiple cookies
      // extract it and add the cookie to the list
      parseCookie(data.substr(oldPos, pos - oldPos));
      
      // update pos (+1 to skip ';')
      oldPos = pos + 1;
    }
  }
}

void
cgicc::CgiEnvironment::parseCookie(const std::string& data)
{
  // find the '=' separating the name and value
  std::string::size_type pos = data.find("=", 0);

  // if no '=' was found, return
  if(std::string::npos == pos)
    return;

  // skip leading whitespace - " \f\n\r\t\v"
  std::string::size_type wscount = 0;
  std::string::const_iterator data_iter;
  
  for(data_iter = data.begin(); data_iter != data.end(); ++data_iter,++wscount)
    if(0 == std::isspace(*data_iter))
      break;			
  
  // Per RFC 2091, do not unescape the data (thanks to afm@othello.ch)
  std::string name 	= data.substr(wscount, pos - wscount);
  std::string value 	= data.substr(++pos);

  fCookies.push_back(HTTPCookie(name, value));
}

// Read in all the environment variables
void
cgicc::CgiEnvironment::readEnvironmentVariables(CgiInput *input)
{
  fServerSoftware 	= input->getenv("SERVER_SOFTWARE");
  fServerName 		= input->getenv("SERVER_NAME");
  fGatewayInterface 	= input->getenv("GATEWAY_INTERFACE");
  fServerProtocol 	= input->getenv("SERVER_PROTOCOL");

  std::string port 	= input->getenv("SERVER_PORT");
  fServerPort 		= std::atol(port.c_str());

  fRequestMethod 	= input->getenv("REQUEST_METHOD");
  fPathInfo 		= input->getenv("PATH_INFO");
  fPathTranslated 	= input->getenv("PATH_TRANSLATED");
  fScriptName 		= input->getenv("SCRIPT_NAME");
  fQueryString 		= input->getenv("QUERY_STRING");
  fRemoteHost 		= input->getenv("REMOTE_HOST");
  fRemoteAddr 		= input->getenv("REMOTE_ADDR");
  fAuthType 		= input->getenv("AUTH_TYPE");
  fRemoteUser 		= input->getenv("REMOTE_USER");
  fRemoteIdent 		= input->getenv("REMOTE_IDENT");
  fContentType 		= input->getenv("CONTENT_TYPE");

  std::string length 	= input->getenv("CONTENT_LENGTH");
  fContentLength 	= std::atol(length.c_str());

  fAccept 		= input->getenv("HTTP_ACCEPT");
  fUserAgent 		= input->getenv("HTTP_USER_AGENT");
  fRedirectRequest 	= input->getenv("REDIRECT_REQUEST");
  fRedirectURL 		= input->getenv("REDIRECT_URL");
  fRedirectStatus 	= input->getenv("REDIRECT_STATUS");
  fReferrer 		= input->getenv("HTTP_REFERER");
  fCookie 		= input->getenv("HTTP_COOKIE");
  fAcceptLanguageString = input->getenv("HTTP_ACCEPT_LANGUAGE"); 

  // Win32 bug fix by Peter Goedtkindt 
  std::string https 	= input->getenv("HTTPS");
  if(stringsAreEqual(https, "on"))
    fUsingHTTPS = true;
  else
    fUsingHTTPS = false;


}

void
cgicc::CgiEnvironment::save(const std::string& filename) 	const
{
  std::ofstream file( filename.c_str(), std::ios::binary |std::ios::out );

  if( ! file )
    throw std::runtime_error("I/O error");

  writeLong(file, fContentLength);
  writeLong(file, fServerPort);
  writeLong(file, (unsigned long) usingHTTPS());

  writeString(file, fServerSoftware);
  writeString(file, fServerName);
  writeString(file, fGatewayInterface);
  writeString(file, fServerProtocol);
  writeString(file, fRequestMethod);
  writeString(file, fPathInfo);
  writeString(file, fPathTranslated);
  writeString(file, fScriptName);
  writeString(file, fQueryString);
  writeString(file, fRemoteHost);
  writeString(file, fRemoteAddr);
  writeString(file, fAuthType);
  writeString(file, fRemoteUser);
  writeString(file, fRemoteIdent);
  writeString(file, fContentType);
  writeString(file, fAccept);
  writeString(file, fUserAgent);
  writeString(file, fRedirectRequest);
  writeString(file, fRedirectURL);
  writeString(file, fRedirectStatus);
  writeString(file, fReferrer);
  writeString(file, fCookie);
  
  if(stringsAreEqual(fRequestMethod, "post") || stringsAreEqual(fRequestMethod, "put"))
    writeString(file, fPostData);

  if(file.bad() || file.fail())
    throw std::runtime_error("I/O error");

  file.close();
}

void
cgicc::CgiEnvironment::restore(const std::string& filename)
{
  std::ifstream file( filename.c_str(), std::ios::binary | std::ios::in );

  if( ! file )
    throw std::runtime_error("I/O error");

  file.flags(file.flags() & std::ios::skipws);

  fContentLength 	= readLong(file);
  fServerPort 		= readLong(file);
  fUsingHTTPS 		= (bool) readLong(file);

  fServerSoftware 	= readString(file);
  fServerName 		= readString(file);
  fGatewayInterface 	= readString(file);
  fServerProtocol 	= readString(file);
  fRequestMethod 	= readString(file);
  fPathInfo 		= readString(file);
  fPathTranslated 	= readString(file);
  fScriptName 		= readString(file);
  fQueryString 		= readString(file);
  fRemoteHost 		= readString(file);
  fRemoteAddr 		= readString(file);
  fAuthType 		= readString(file);
  fRemoteUser 		= readString(file);
  fRemoteIdent 		= readString(file);
  fContentType 		= readString(file);
  fAccept 		= readString(file);
  fUserAgent 		= readString(file);
  fRedirectRequest 	= readString(file);
  fRedirectURL 		= readString(file);
  fRedirectStatus	= readString(file);
  fReferrer 		= readString(file);
  fCookie 		= readString(file);
  
  if(stringsAreEqual(fRequestMethod, "post") || stringsAreEqual(fRequestMethod, "put"))
    fPostData = readString(file);

  file.close();

  fCookies.clear();
  fCookies.reserve(10);
  parseCookies();
}

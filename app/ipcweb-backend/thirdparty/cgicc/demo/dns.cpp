/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: dns.cpp,v 1.25 2009/01/03 17:26:43 sebdiaz Exp $
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

/*! \file dns.cpp
 * \brief A WWW to DNS gateway
 *
 * A sample CGI application using the GNU %cgicc library.  This script 
 * allows users to lookup the IP address corresponding to a hostname, 
 * or vice-versa.
 */

#include <cstdlib>
#include <new>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <stdio.h>
#include <memory.h>
#include "cgicc/CgiDefs.h"
#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#if HAVE_SYS_UTSNAME_H
#  include <sys/utsname.h>
#endif

#if HAVE_SYS_TIME_H
#  include <sys/time.h>
#endif

#ifdef WIN32
#  include <winsock2.h>
#else
#  include <sys/types.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#  include <netdb.h>
#endif /* WIN32 */

#include "styles.h"

using namespace std;
using namespace cgicc;

// DNS gateway cgi
int
main(int /*argc*/, 
     char ** /*argv*/)
{

  try {
#if HAVE_GETTIMEOFDAY
    timeval start;
    gettimeofday(&start, NULL);
#endif

    Cgicc cgi;
    
    cout << HTTPHTMLHeader() << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html().set("lang","en").set("dir","ltr") << endl;
    
    // Set up the page; I will put in lfs to ease reading of the
    // produced HTML. These are optional, and except in <PRE>
    // tags have no effect on HTML appearance.
    cout << head() << endl;

    // Output the style sheet portion of the header
    cout << style() << comment() << endl;
    cout << styles;
    cout << comment() << style() << endl;

    cout << title("DNS Gateway") << endl;
    cout << head() << endl;
    
    cout << h1() << "GNU cgi" << span("cc").set("class","red")
	 << " DNS Gateway" << h1() << endl;
  
    form_iterator ip = cgi.getElement("ip");
    form_iterator name = cgi.getElement("hostname");

    if(ip != (*cgi).end()) {
      cout << h3() << "Query results for " << **ip << h3() << endl;
      
      u_long addr;
      struct hostent *hp;
      char **p;
      
      if((int)(addr = inet_addr((**ip).c_str())) == -1) {
	cout << cgicc::div().set("class", "notice") << endl
	     << strong(span("ERROR").set("class","red"))
	     << " - IP address must be of the form x.x.x.x"
	     << endl << cgicc::div() << endl;
      }
      else {
	hp = gethostbyaddr((char*)&addr, sizeof (addr), AF_INET);
	if(hp == NULL) {
	  cout << cgicc::div().set("class", "notice") << endl
	       << strong(span("ERROR").set("class","red")) 
	       << " - Host information for " << em((**ip)) << " not found."
	       << endl << cgicc::div() << endl;
	}
	else {
	  for(p = hp->h_addr_list; *p != 0; p++) {
	    struct in_addr in;
	    //char **q;
	    
	    (void) memcpy(&in.s_addr, *p, sizeof(in.s_addr));
	    
	    cout << cgicc::div().set("class", "notice") << endl
		 << span(inet_ntoa(in)).set("class","blue") 
		 << " - " << ' ' << hp->h_name;
	    //for(q = hp->h_aliases; *q != 0; q++)
	    //	    cout << *q << ' ';
	    cout << endl << cgicc::div() << endl;
	  }
	}
      }
    }
    

    if(name != (*cgi).end()) {
      cout << h3() << "Query results for " << **name << h3() << endl;
      
      struct hostent *hp;
      char **p;
      
      hp = gethostbyname((**name).c_str());
      if(hp == NULL) {
	cout << cgicc::div().set("class", "notice") << endl
	     << strong(span("ERROR").set("class","red"))
	     << " - Host information for " << em(**name) << " not found."
	     << endl << cgicc::div() << endl;
      }
      else {
	for(p = hp->h_addr_list; *p != 0; p++) {
	  struct in_addr in;
	  //char **q;
	  
	  (void) memcpy(&in.s_addr, *p, sizeof(in.s_addr));
	  
	  cout << cgicc::div().set("class", "notice") << endl
	       << inet_ntoa(in) << " - " << ' ' 
	       << span(hp->h_name).set("class","blue");
	  //	for(q = hp->h_aliases; *q != 0; q++)
	  //	  cout << *q << ' ';
	  cout << endl << cgicc::div() << endl;
	}
      }
    }
    
    cout << p("Please enter an IP address or a hostname.") << endl;
    
    cout << table() << endl;
    
    cout << "<form method=\"post\" action=\""
	 << cgi.getEnvironment().getScriptName() << "\">" << endl;
    
    cout << tr() << endl;
    cout << td(strong("IP Address: ")).set("class", "title") << endl;
    cout << td().set("class", "data") << "<input type=\"text\" name=\"ip\"";
    if(ip != (*cgi).end())
      cout << " value=\"" << **ip << "\">";
    else
      cout << ">";
    cout << td() << tr() << "</form>" << endl;
    
    cout << "<form method=\"post\" action=\""
	 << cgi.getEnvironment().getScriptName() << "\">" << endl;
    
    cout << tr() << endl;
    cout << td(strong("Hostname: ")).set("class", "title") << endl;
    cout << td().set("class", "data") 
	 << "<input type=\"text\" name=\"hostname\"";
    if(name != (*cgi).end())
      cout << " value=\"" << **name << "\">";
    else
      cout << ">";
    cout << td() << tr() << endl;
    cout << "</form>" << table() << p() << endl;
    
    // Now print cout a footer with some fun info
    cout << hr(set("class","half")) << endl;
    cout << cgicc::div().set("align","center").set("class","smaller") << endl;
    cout << "GNU cgi" << span("cc").set("class","red") << " v"
	 << cgi.getVersion() << br() << endl;
    cout << "Compiled at " << cgi.getCompileTime() 
	 << " on " << cgi.getCompileDate() << br() << endl;
    
    cout << "Configured for " << cgi.getHost();  
    // I don't know if everyone has uname...
#if HAVE_UNAME
    struct utsname info;
    if(uname(&info) != -1) {
      cout << ". Running on " << info.sysname;
      cout << ' ' << info.release << " (";
      cout << info.nodename << ')' << endl;
    }
#else
    cout << '.' << endl;
#endif
    
#if HAVE_GETTIMEOFDAY
    // Information on this query
    timeval end;
    gettimeofday(&end, NULL);
    long us = ((end.tv_sec - start.tv_sec) * 1000000)
      + (end.tv_usec - start.tv_usec);

    cout << br() << "Total time for request = " << us << " us";
    cout << " (" << static_cast<double>(us/1000000.0) << " s)";
#endif
    
    // End of document
    cout << cgicc::div() << endl;
    cout << body() << html() << endl;

    return EXIT_SUCCESS;
  }

  catch(const std::exception& e) {
    return EXIT_FAILURE;
  }
}

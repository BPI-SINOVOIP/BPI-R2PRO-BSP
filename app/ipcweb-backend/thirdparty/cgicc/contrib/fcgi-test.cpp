/*
 * $Id: fcgi-test.cpp,v 1.5 2007/07/02 18:48:19 sebdiaz Exp $ 
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth
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

#include <exception>
#include <iostream>

#include <unistd.h>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPHTMLHeader.h"
#include "cgicc/HTMLClasses.h"

#include "FCgiIO.h"

using namespace std;
using namespace cgicc;

int 
main(int /*argc*/, 
     const char **/*argv*/, 
     char **/*envp*/)
{
  unsigned count = 0;

  FCGX_Request request;

  FCGX_Init();
  FCGX_InitRequest(&request, 0, 0);

  while(FCGX_Accept_r(&request) == 0) {

    try {
      FCgiIO IO(request);
      Cgicc CGI(&IO);

      // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
      IO << HTTPHTMLHeader() << HTMLDoctype( HTMLDoctype::eStrict ) << endl
         << html().set( "lang", "en" ).set( "dir", "ltr" ) << endl;

      // Set up the page's header and title.
      IO << head() << endl
         << title() << "GNU cgicc v" << CGI.getVersion() << title() << endl
         << head() << endl;

      // Start the HTML body
      IO << body() << endl;

      // Print out a message
      IO << h1("Cgicc/FastCGI Test") << endl
         << "PID: " << getpid() << br() << endl
         << "count: " << count++ << br() << endl;

      IO  << "Form Elements:" << br() << endl;

      for(const_form_iterator i = CGI.getElements().begin(); 
	  i != CGI.getElements().end(); ++i )
        IO << i->getName() << " = " << i->getValue() << br() << endl;

      // Close the document
      IO << body() << html();
    }
    catch(const exception&) {
      // handle error condition
    }

    FCGX_Finish_r(&request);
  }

  return 0;
}

/* -*-mode:c++; c-file-style: "gnu";-*- */
/*
 *  $Id: upload.cpp,v 1.13 2007/07/02 18:48:19 sebdiaz Exp $
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

/*! \file upload.cpp
 * \brief File upload demo
 *
 * Tests and demonstrates how to handle uploaded files using the 
 * GNU %cgicc library.
 */

#include <new>
#include <string>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <cstdlib>

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

#include "styles.h"

using namespace std;
using namespace cgicc;

// Print the form for this CGI
void
printForm(const Cgicc& cgi)
{
  cout << "<form method=\"post\" action=\"" 
       << cgi.getEnvironment().getScriptName() 
       << "\" enctype=\"multipart/form-data\">" << endl;
    
  cout << "<table>" << endl;

  cout << "<tr><td class=\"title\">Send a file</td>"
       << "<td class=\"form\">"
       << "<input type=\"file\" name=\"userfile\" accept=\"text/plain\" />"
       << "</td></tr>" << endl;

  cout << "<tr><td class=\"title\">Upload Redirection</td>"
       << "<td class=\"form\">"
       << "<input type=\"checkbox\" name=\"redirect\" />"
       << "Bounce uploaded file back to browser"
       << "</td></tr>" << endl;

  cout << "</table>" << endl;

  cout << "<div class=\"center\"><p>"
       << "<input type=\"submit\" name=\"submit\" value=\"Send the file\" />"
       << "<input type=\"reset\" value=\"Nevermind\" />"
       << "</p></div></form>" << endl;
}

// Main Street, USA
int
main(int /*argc*/, 
     char ** /*argv*/)
{
  try {
#if HAVE_GETTIMEOFDAY
    timeval start;
    gettimeofday(&start, NULL);
#endif

    // Create a new Cgicc object containing all the CGI data
    Cgicc cgi;

    // Redirect output, if desired
    if(cgi.queryCheckbox("redirect")) {
      const_file_iterator file = cgi.getFile("userfile");

      // Only redirect a valid file
      if(file != cgi.getFiles().end()) {
	cout << HTTPContentHeader(file->getDataType());
	file->writeToStream(cout);

	return EXIT_SUCCESS;
      }
    }
    
    // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
    cout << HTTPHTMLHeader() << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html().set("lang", "en").set("dir", "ltr") << endl;

    // Set up the page's header and title.
    // I will put in lfs to ease reading of the produced HTML. 
    cout << head() << endl;

    // Output the style sheet portion of the header
    cout << style() << comment() << endl;
    cout << styles;
    cout << comment() << style() << endl;

    cout << title() << "GNU cgicc v" << cgi.getVersion() 
	 << " File Upload Test" << title() << endl;

    cout << head() << endl;
    
    // Start the HTML body
    cout << body() << endl;

    cout << h1() << "GNU cgi" << span("cc").set("class","red")
	 << " v"<< cgi.getVersion() << " File Upload Test" 
	 << h1() << endl;

    // Get a pointer to the environment
    const CgiEnvironment& env = cgi.getEnvironment();
    
    // Generic thank you message
    cout << comment() << "This page generated by cgicc for "
	 << env.getRemoteHost() << comment() << endl;
    cout << h4() << "Thanks for using cgi" << span("cc").set("class", "red") 
	 << ", " << env.getRemoteHost() 
	 << '(' << env.getRemoteAddr() << ")!" << h4() << endl;  
    
    // Show the uploaded file
    const_file_iterator file;
    file = cgi.getFile("userfile");
				
    if(file != cgi.getFiles().end()) {

      cout << table() << endl;
      
      cout << tr() << td("Name").set("class","title")
	   << td(file->getName()).set("class","data") << tr() << endl;
      
      cout << tr() << td("Data Type").set("class","title")
	   << td(file->getDataType()).set("class","data") << tr() << endl;
      
      cout << tr() << td("Filename").set("class","title") 
	   << td(file->getFilename()).set("class","data") << tr() << endl;

      cout << tr() << td("Data Length").set("class","title") 
	   << td().set("class","data") << file->getDataLength() 
	   << td() << tr() << endl;
      
      cout << tr() << td("File Data").set("class","title")
	   << td().set("class","data") << pre();
      file->writeToStream(cout);
      cout << pre() << td() << tr() << endl;
      
      cout << table() << endl;
    }

    // Print out the form to do it again
    cout << br() << endl;
    printForm(cgi);
    cout << hr().set("class", "half") << endl;

    // Information on cgicc
    cout << cgicc::div().set("align","center").set("class","smaller") << endl;
    cout << "GNU cgi" << span("cc").set("class","red") << " v";
    cout << cgi.getVersion() << br() << endl;
    cout << "Compiled at " << cgi.getCompileTime();
    cout << " on " << cgi.getCompileDate() << br() << endl;

    cout << "Configured for " << cgi.getHost();  
#if HAVE_UNAME
    struct utsname info;
    if(uname(&info) != -1) {
      cout << ". Running on " << info.sysname;
      cout << ' ' << info.release << " (";
      cout << info.nodename << ")." << endl;
    }
#else
    cout << "." << endl;
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

    // No chance for failure in this example
    return EXIT_SUCCESS;
  }

  // Did any errors occur?
  catch(const std::exception& e) {

    // This is a dummy exception handler, as it doesn't really do
    // anything except print out information.

    // Reset all the HTML elements that might have been used to 
    // their initial state so we get valid output
    html::reset(); 	head::reset(); 		body::reset();
    title::reset(); 	h1::reset(); 		h4::reset();
    comment::reset(); 	td::reset(); 		tr::reset(); 
    table::reset();	cgicc::div::reset(); 	p::reset(); 
    a::reset();		h2::reset(); 		colgroup::reset();

    // Output the HTTP headers for an HTML document, and the HTML 4.0 DTD info
    cout << HTTPHTMLHeader() << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html().set("lang","en").set("dir","ltr") << endl;

    // Set up the page's header and title.
    // I will put in lfs to ease reading of the produced HTML. 
    cout << head() << endl;

    // Output the style sheet portion of the header
    cout << style() << comment() << endl;
    cout << "body { color: black; background-color: white; }" << endl;
    cout << "hr.half { width: 60%; align: center; }" << endl;
    cout << "span.red, strong.red { color: red; }" << endl;
    cout << "div.notice { border: solid thin; padding: 1em; margin: 1em 0; "
	 << "background: #ddd; }" << endl;

    cout << comment() << style() << endl;

    cout << title("GNU cgicc exception") << endl;
    cout << head() << endl;
    
    cout << body() << endl;
    
    cout << h1() << "GNU cgi" << span("cc", set("class","red"))
	 << " caught an exception" << h1() << endl; 
  
    cout << cgicc::div().set("align","center").set("class","notice") << endl;

    cout << h2(e.what()) << endl;

    // End of document
    cout << cgicc::div() << endl;
    cout << hr().set("class","half") << endl;
    cout << body() << html() << endl;
    
    return EXIT_SUCCESS;
  }
}

/*
 * $Id: nph-login.cpp,v 1.8 2007/07/02 18:48:19 sebdiaz Exp $
 *
 *  Copyright (C) 1996 - 2004 Stephen F. Booth
 *  Copyright (C) 2001 Peter Goedtkindt
 *                2007 Sebastien DIAZ <sebastien.diaz@gmail.com>
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

/* 
 * In this example, we will control the HTTPNPH header so that the user
 * will need to provide authentication before the cgi does something.
 * So far, this CGI has only been tested on IIS 5 servers and using
 * browsers IE 5 (Mac OS 9, MacOS X, Windows), Netscape 4.7 (Windows,
 * Linux, Mac), Omniweb 4 (Mac) The ask for user authentication, the
 * cgi issues a 403 http response code.  
 *
 * IMPORTANT: To be able to control the response type HTTP header, the
 * name of the cgi executable needs to start with "nph-". The cgi is
 * then fully responsible for the creation of _ALL_ http headers.
 *
 * IMPORTANT2: Once a valid username/password has been accepted, the
 * browser will send the pair with each request until you quit your
 * browser.  
 *
 * IMPORTANT3: On Windows servers, make certain your webserver is
 * configured to allow basic authentication.
 */

#include <exception>
#include <iostream>

#include "cgicc/Cgicc.h"
#include "cgicc/HTTPResponseHeader.h"
#include "cgicc/HTMLClasses.h"

using namespace std;
using namespace cgicc;

int
main(int /*argc*/, 
     char ** /*argv*/)
{
  try {
    Cgicc cgi;
    const CgiEnvironment& env = cgi.getEnvironment();
    string remoteuser = env.getRemoteUser();
    string serversw = env.getServerSoftware();
    string clientsw = env.getUserAgent();
    string authtype = env.getAuthType();

    if(remoteuser.empty()) {
      if (serversw.find("Microsoft") != string::npos 
	  && clientsw.find("Win") != string::npos) {
	/*
	  Server and client are running on Microsoft OS, so we
	  probably can request NTLM authentication; the last test was
	  needed to prevent IE on Mac's from using NTLM, because it
	  seems to be broken on Macs 
	*/

     cout << HTTPResponseHeader("HTTP/1.1", 401, "Unauthorized")
	  .addHeader("WWW-Authenticate", "NTLM")
	  .addHeader("WWW-Authenticate",  "Basic realm=\"cgicc\"");
  
        /*
	  There is a bug in all version of Microsoft Internet Explorer
	  at least up to 5.5 by which the NTLM authentication scheme
	  MUST be declared first or it won't be selected. This goes
	  against RFC 2616, which recites "A user agent MUST choose to
	  use the strongest auth- scheme it understands" and NTLM, while
	  broken in many ways, is still worlds stronger than Basic.
        */
      }
      else {
	// we're not chatting fully MS: only support basic
        cout << HTTPResponseHeader("HTTP/1.1", 401, "Unauthorized")
	  .addHeader("WWW-Authenticate", "Basic realm=\"cgicc\"");
      }
      // do not add html data: browsers should not display this anyway
      //  they should request user/password from the user and re-emit
      //  the same request, only with the authentification info added
      //  to the request 
      cout << HTMLDoctype(HTMLDoctype::eStrict) << endl;
      cout << html().set("lang", "EN").set("dir", "LTR") << endl;
    
      cout << head() << endl;
      cout << title("401 Authorization Required")  << endl;
      cout << head() << endl;
    
      cout << body() << endl;
      cout << h1("401 Authorization Required") << endl;
      cout << p() << "This server could not verify that you are "
	   << "authorized to access the document requested. Either you "
	   << "supplied the wrong credentials (e.g., bad password), or "
	   << "your browser doesn't understand how to supply the "
	   << "credentials required." << p();

      cout << hr() << endl;
      cout << address() << "GNU cgicc \"server\" version " << cgi.getVersion()
	   << address() << endl;

      return 0;
    }
  
    // Output the HTTP headers 200 OK header for an HTML document, and
    // the HTML 4.0 DTD info
    cout << HTTPResponseHeader(env.getServerProtocol(), 200 ,"OK")
      .addHeader("Content-Type", "text/html");
    cout << HTMLDoctype(HTMLDoctype::eStrict) << endl;
    cout << html().set("lang", "EN").set("dir", "LTR") << endl;

    // Set up the page's header and title.
    cout << head() << endl;
    cout << title() << "GNU cgicc v" << cgi.getVersion() << title() << endl;
    cout << head() << endl;
    
    // Start the HTML body
    cout << body() << endl;

    // Print out a message
    cout << "Hello " << env.getRemoteUser() 
	 << " your login was accepted" << br() << endl;
    cout << "You were authenticated using authentication scheme : " 
	 << env.getAuthType() << br() << endl;
    cout << "Your server software is :" << serversw << br() << endl;
    cout << "Your browser software is :" << clientsw << br() << endl;
    // Close the document
    cout << body() << html();


  }
  
  catch(const exception& e) {
    // handle error condition
  }
  
  return 0;
}

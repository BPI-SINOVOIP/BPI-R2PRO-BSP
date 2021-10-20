/*
 *  $Id: styles.h,v 1.3 2007/07/02 18:48:19 sebdiaz Exp $
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
 */
#ifndef _STYLES_H_
#define _STYLES_H_ 1

#include <string>

const std::string styles = 
"body { color: black; background: white; }\n"
"span.red { color:red; }\n"
"hr.half { width: 60%; margin-left: auto; margin-right: auto; }\n"
"div.center { text-align: center; }\n"
"div.notice { border: solid thin; padding: 1em; margin: 1em 0; "
"background: #ddd; text-align: center; }"
"table { width: 90%; margin-left: auto; margin-right: auto; }\n"
"tr.title, td.title { color: white; background: black; font-weight: bold; "
"text-align: center; }\n"
"tr.data, td.data { background: #ddd; }\n"
"td.form { background: #ddd; text-align: center; }\n"
;

#endif /* _STYLES_H_ */

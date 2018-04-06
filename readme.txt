/*
 *     Copyright (C) 2014-2018 Sumandeep Banerjee
 * 
 *     This program is free software: you can redistribute it and/or modify
 *     it under the terms of the GNU Lesser General Public License as published by
 *     the Free Software Foundation, either version 3 of the License, or
 *     (at your option) any later version.
 * 
 *     This program is distributed in the hope that it will be useful,
 *     but WITHOUT ANY WARRANTY; without even the implied warranty of
 *     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *     GNU General Public License for more details.
 * 
 *     You should have received a copy of the GNU Lesser General Public License
 *     along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/* 
 * File:   
 * Author: sumandeep
 * Email:  sumandeep.banerjee@gmail.com
*/

-------------------
Image Search Engine
-------------------

1. How to build the project
	a. Requires OpenCV 2.4.9, CMake 2.9 or later
	b. Run in the base directory (directory containing the file CMakeLists.txt)
		$ CMake . 
		$ Make
	c. Also included is the NetBeans project, requires NetBeans IDE 8.0.1 or later
	d. If successfull, you should get three executables ImageSearch_test, ImageSearch_server and ImageSearch_client

2. ImageSearch_test is used to build the vocabulary table from the initial corpus of images.
	$ ./ImageSearch_test 
	Follow the command line instructions.

3. ImageSearch_server is run using the config file (ImageSearch_config.xml). It loads the database, and keeps running, ready for queries from the client.
	$ ./ImageSearch_server
	Follow the command line instructions.

4. ImageSearch_client could be run from the terminal or via python script. It communicates with the local server via UNIX socket and returns the result
	$ ./ImageSearch_client
	Follow the command line instructions.
	

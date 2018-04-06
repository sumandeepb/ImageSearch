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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include "Common.h"

using namespace std;

const double INF = 1.0E10;
const double EPSILON = 1E-6;

const int MAX_WIDTH = 640;
const int MAX_HEIGHT = 480;

const int NUM_TOP_MATCHES = 5;

const std::string TEMP_FOLDER = "temp";
const std::string IMAGE_FOLDER = "image";
const std::string DESCR_FOLDER = "descr";
const std::string HASH_FOLDER = "hash";

const std::string FILE_FORMAT = ".xml";
const std::string MAIN_FILE = "_main";
const std::string VOCAB_FILE = "_vocab";
const std::string HASH_FILE = "_hash";

void LogData( const char *szFormat, ... )
{
	va_list args;
	va_start( args, szFormat );
	vprintf( szFormat, args );
	va_end( args );
	fflush( stdout );
}

// parse list file
int ParseListFile( const std::string &strListFile, std::vector<std::string> &vecImageNames )
{
	// open file stream
	ifstream fs( strListFile.c_str() );
	if( !fs.is_open() )
	{
		cerr << "Failed to open list file." << endl;
		return -1;
	}

	// read number of images in DB List
	unsigned int N;
	fs >> N;
	vecImageNames.resize( N );

	// read image file names and create list
	for( unsigned int i = 0; i < N; i++ )
	{
		string strTemp;
		fs >> strTemp;
		vecImageNames[i] = strTemp;
	}

	return 0;
}

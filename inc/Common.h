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

#pragma once

#include <string>
#include <vector>

// infinity
extern const double INF;

// infinitesimal
extern const double EPSILON;

// maximum dimentions of image (larger images are scaled down)
extern const int MAX_WIDTH;
extern const int MAX_HEIGHT;

// number of top matches that are tested for geometric validation
extern const int NUM_TOP_MATCHES;

extern const std::string TEMP_FOLDER;	// name of temp sub folder
extern const std::string IMAGE_FOLDER;	// sub folder for storing images
extern const std::string DESCR_FOLDER;	// sub folder for storing descriptors
extern const std::string HASH_FOLDER;	// sub folder for storing hash maps

extern const std::string FILE_FORMAT;	// format of data files (.xml or .yaml)
extern const std::string MAIN_FILE;		// DB file postfix for main file
extern const std::string VOCAB_FILE;	// DB file postfix for vocab file
extern const std::string HASH_FILE;		// DB file postfix for hash file

void LogData( const char *szFormat, ... ); // function to log data (verbose in DEBUG mode)

int ParseListFile( const std::string &strListFile,
	std::vector<std::string> &vecImageNames ); // function to parse list file

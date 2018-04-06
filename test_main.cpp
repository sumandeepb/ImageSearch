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
#include <iomanip>
#include "Common.h"
#include "SearchEngine.h"

using namespace std;
using namespace cv;

// display command line help
void printHelp( const string& strAppName )
{
	cout << String( 40, '-' ) << endl;
	cout << strAppName << " usage options: " << endl;
	cout << String( 40, '-' ) << endl << endl;

	cout << "Training Test: " << endl;
	cout << String( 15, '-' ) << endl;
	cout << strAppName << " t dbpath dbname trainingpath" << endl << endl;

	cout << "Search Test: " << endl;
	cout << String( 15, '-' ) << endl;
	cout << strAppName << " s dbpath dbname querypath" << endl << endl;

	cout << "dbpath         - path to database folder location" << endl;
	cout << "dbname         - name of the database file" << endl;
	cout << "trainingpath   - path location of training files" << endl;
	cout << "querypath      - path location of validation files" << endl << endl;
}

// sample test application for search engine training and searching
int main( int argc, char* argv[] )
{
	unsigned int posSplit = string( argv[0] ).find_last_of( "/\\" );
	string strAppName = string( argv[0] ).substr( posSplit + 1 );
	
	if( 5 != argc )
	{
		printHelp( strAppName );
		return -1;
	}

	if( 0 == strcmp( "t", argv[1] ) ) // training test routine
	{
		CSearchEngine cCoverSearch;

		// set parameters from command line arguments
		const string strDBPath = argv[2];
		const string strDBName = argv[3];
		const string strTrainImgPath = argv[4];

		// parse image list file to extract source image names for the training set
		vector<string> vecTrainImagList;
		if ( ParseListFile( strTrainImgPath + "/imagelist.txt", vecTrainImagList ) )
		{
			cerr << "Failed to parse imagelist file." << endl;
			return -1;
		}

		// create list of names for the image records
		unsigned int i = 0;
		stringstream strBuffer;
		vector<string> vecImageList;
		for( vector<string>::iterator it = vecTrainImagList.begin(); it != vecTrainImagList.end(); it++, i++ )
		{
			(*it) = strTrainImgPath + "/" + (*it);
			strBuffer << "i" << setw(7) << setfill('0') << i;
			vecImageList.push_back( strBuffer.str() );
			strBuffer.str( string() );
		}

		// test database creation
		cout << "Creating database: " << strDBName << "...";
		if ( cCoverSearch.CreateDB( strDBPath, strDBName ) )
		{
			cerr << "Failed to create database." << endl;
			return -1;
		}
		cout << "success\n";

		// loading images and and descriptor extraction
		cout <<  "Adding images to database...\n";
		if ( cCoverSearch.AddFileList( vecTrainImagList, vecImageList ) )
		{
			cerr << "Failed to add file to image database." << endl;
			return -1;
		}
		cout << vecImageList.size() << " file(s) succeeded\n";

		// save data base to disk
		cout << "Saving image records...";
		if ( cCoverSearch.SaveImageDB() )
		{
			cerr << "Failed to save image records." << endl;
			return -1;
		}
		cout << "success\n";

		// test tree building
		cout << "Building vocabulary tree...";
		if ( cCoverSearch.BuildVocabTree( 10, 5 ) )
		{
			cerr << "Failed to build vocabulary tree." << endl;
			return -1;
		}
		cout << "success\n";

		// save vocab tree
		cout << "Saving vocabulary tree...";
		if ( cCoverSearch.SaveVocabTree() )
		{
			cerr << "Failed to save vocabulary tree." << endl;
			return -1;
		}
		cout << "success\n";

#if HIST_SEARCH
		// build work histogram based hash map
		cout << "Building hash table...";
		if ( cCoverSearch.BuildHashTable() )
		{
			cerr << "Failed to build hash table." << endl;
			return -1;
		}
		cout << "success\n";

		// save hash map to file
		cout << "Saving hash table...";
		if ( cCoverSearch.SaveHashTable() )
		{
			cerr << "Failed to save hash table." << endl;
			return -1;
		}
		cout << "success\n";
#endif
	}
	else if( 0 == strcmp( "s", argv[1] ) ) // search test routine
	{
		CSearchEngine cCoverSearch;

		// set parameters from command line arguments
		const string strDBPath = argv[2];
		const string strDBName = argv[3];
		const string strQueryImgPath = argv[4];

		// load search database (image records, vocabulary table, hash map)
		cout << "Loading search engine...";
		if( cCoverSearch.LoadDB( strDBPath, strDBName, false ) )
		{
			cerr << "Failed to load search database." << endl;
			return -1;
		}
		cout << "success\n";

		// run query loop
		while(1)
		{
			string strQueryName;

			// take query filename as input
			cout << "\nEnter query image name (exit to quit): ";
			cin >> strQueryName;
			if( "exit" == strQueryName )
				break;

			// search for the best matching image from the database
			cout << "Searching database...\n";
			vector< string > vecBestMatches;
            cCoverSearch.SearchDB( strQueryImgPath + "/" + strQueryName, vecBestMatches );
            string strResult = *(vecBestMatches.begin());
            cout << "Result:\n" << strResult << "\n";
		}
	}
	else // invalid option
	{
		printHelp( strAppName );
		return -1;
	}

	return 0;
}

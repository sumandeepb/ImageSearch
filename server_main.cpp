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
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <opencv2/opencv.hpp>

#include "SearchEngine.h"

using namespace std;
using namespace cv;

// display command line help
void printHelp( const string& strAppName )
{
	cout << string( 40, '-' ) << endl;
	cout << strAppName << " usage options: " << endl;
	cout << string( 40, '-' ) << endl << endl;
	cout << strAppName << " configfile" << endl << endl;

	cout << "configfile         - .xml config file" << endl << endl;
}

/*
 * Image search server
*/
int main( int argc, char* argv[] )
{
	unsigned int posSplit = string( argv[0] ).find_last_of( "/\\" );
	string strAppName = string( argv[0] ).substr( posSplit + 1 );
	
	if( 2 != argc )
	{
		printHelp( strAppName );
		return -1;
	}

	cout << "Starting Image Search Server..." << endl;
    
    // loading config file
    cout << "Loading config file: " << argv[1] << endl;
    FileStorage fs( argv[1], FileStorage::READ );
    if( !fs.isOpened() )
    {
        cerr << "Failed to load config file" << endl;
        return -1;
    }
	
    string strDBPath, strDBName, strSockName;
    fs["dbpath"] >> strDBPath;
    fs["dbname"] >> strDBName;
    fs["socket"] >> strSockName;
    fs.release();
    
    CSearchEngine cCoverSearch;
    
    // try loading the database
    cout << "Database Path: " << strDBPath << endl;
    cout << "Database Name: " << strDBName << endl;
    cout << "Loading image database..." << endl;
    if( 0 != cCoverSearch.LoadDB( strDBPath, strDBName, false ) )
    {
        // try creating the database
        if( 0 != cCoverSearch.CreateDB( strDBPath, strDBName ) )
        {
            cerr << "Failed to create image database" << endl;
            return -1;
        }
        else
        {
            cout << "Created image database" << endl;
        }
    }
    else
    {
        cout << "Loaded image database" << endl;
    }
    
    cout << "Setting up listening socket..." << endl;

    // create server side socket
    int nSocketID = socket(AF_UNIX, SOCK_STREAM, 0);
    if(nSocketID < 0)
    {
        cout << "Failed to create server socket" << endl;
        return -1;
    }
    
    unlink( strSockName.c_str() );

    // bind socket address to server
    struct sockaddr_un sServSockAddr;
    memset( &sServSockAddr, 0, sizeof(struct sockaddr_un) );
    sServSockAddr.sun_family = AF_UNIX;
    strncpy( sServSockAddr.sun_path, strSockName.c_str(), 108 );

    if( 0 != bind( nSocketID, (struct sockaddr *)&sServSockAddr, sizeof(struct sockaddr_un) ) )
    {
        cout << "Failed to bind socket address to server" << endl;
        return -1;
    }

    if( 0 != listen( nSocketID, 5 ) )
    {
        cout << "Failed to setup listen socket" << endl;
        return -1;
    }
    
    cout << "Image Search Server is up and running..." << endl;
 
    /* fork a daemon */
    
    // check for incoming connection on socket
    socklen_t nAddrLength;
    struct sockaddr_un sCliSockAddr;
    while( true )
    {
        nAddrLength = sizeof(sCliSockAddr);
        int nConnectionID = accept( nSocketID, (struct sockaddr *)&sCliSockAddr, &nAddrLength );
        // valid connection accepted
        if( nConnectionID >= 0)
        {
            // parse client message
            char szMsgBuff[256];
            int nBytes = read(nConnectionID, szMsgBuff, 256);
            szMsgBuff[nBytes] = 0;
            
            // do message handling
            char szCommand[25];
            szCommand[0] = 0;
            sscanf( szMsgBuff, "%s", szCommand);
            
            if( 0 == strcmp( szCommand, "exit") )
            {
                cout << "Exit command received" << endl;
                nBytes = snprintf( szMsgBuff, 256, "Image Search Server shutting down..." );
                write( nConnectionID, szMsgBuff, nBytes );
                // terminate client connection
                close(nConnectionID);
                break;
            }
            else if( 0 == strcmp( szCommand, "search") )
            {
                cout << "Search command received" << endl;
                
                char szQueryPath[256];
                sscanf( szMsgBuff, "%s%s", szCommand, szQueryPath );
                
                vector< string > vecBestMatches;
                if( 0 != cCoverSearch.SearchDB( szQueryPath, vecBestMatches ) )
                {
                    cout << "Search command failed" << endl;
                    nBytes = snprintf( szMsgBuff, 256, "Search command failed" );
                    write( nConnectionID, szMsgBuff, nBytes );
                }
                else
                {
                    nBytes = snprintf( szMsgBuff, 256, "%s", vecBestMatches.begin()->c_str() );
                    write( nConnectionID, szMsgBuff, nBytes );
                }
            }
            
            // terminate client connection
            close(nConnectionID);
        }
    }

    cout << "Stopping Image Search Server..." << endl;
    
    close( nSocketID );
    unlink( strSockName.c_str() ); 
	
    return 0;
}

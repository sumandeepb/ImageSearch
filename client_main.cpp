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
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <iostream>
#include <string>

using namespace std;

// display command line help
void printHelp( const string& strAppName )
{
	cout << string( 40, '-' ) << endl;
	cout << strAppName << " usage options: " << endl;
	cout << string( 40, '-' ) << endl << endl;

    cout << "Search Query:" << endl;
	cout << strAppName << " -s queryfile" << endl << endl;

    cout << "Stop Server:" << endl;
	cout << strAppName << " -q" << endl << endl;

	cout << "queryfile      - search query file" << endl << endl;
}

int main( int argc, char* argv[] )
{
	unsigned int posSplit = string( argv[0] ).find_last_of( "/\\" );
	string strAppName = string( argv[0] ).substr( posSplit + 1 );
	
	if( argc < 2 )
	{
		printHelp( strAppName );
		return -1;
	}

    //cout << "ImageSearch Client" << endl;
    
    // create client side socket
    int nSocketID = socket(AF_UNIX, SOCK_STREAM, 0);
    if( nSocketID < 0 )
    {
        cout << "Failed to create client socket" << endl;
        return -1;
    }

    // connect to server address
    struct sockaddr_un sServSockAddr;
    memset( &sServSockAddr, 0, sizeof(struct sockaddr_un) );
    sServSockAddr.sun_family = AF_UNIX;
    strncpy( sServSockAddr.sun_path, "./searchsocket", 108 );

    if( 0 != connect( nSocketID, (struct sockaddr *)&sServSockAddr, sizeof(struct sockaddr_un) ) )
    {
        cout << "Failed to connect server socket" << endl;
        return -1;
    }
    
    // 
    if( 0 == strcmp( "-s", argv[1] ) )
    {
        if( 3 != argc )
        {
            printHelp( strAppName );
            return -1;
        }
        
        char szMsgBuffer[256];
        int nNumBytes = snprintf( szMsgBuffer, 256, "search %s", argv[2] );
        write(nSocketID, szMsgBuffer, nNumBytes);
        nNumBytes = read( nSocketID, szMsgBuffer, 256 );
        szMsgBuffer[nNumBytes] = 0;
        cout << "Search Result: " << szMsgBuffer << endl;
    }
    else if( 0 == strcmp( "-q", argv[1] ) )
    {
        if( 2 != argc )
        {
            printHelp( strAppName );
            return -1;
        }
        
        char szMsgBuffer[256];
        int nNumBytes = snprintf( szMsgBuffer, 256, "exit" );
        write( nSocketID, szMsgBuffer, nNumBytes );
        nNumBytes = read( nSocketID, szMsgBuffer, 256 ); 
        szMsgBuffer[nNumBytes] = 0;
        cout << szMsgBuffer << endl;
    }
    else
    {
        printHelp( strAppName );
		return -1;
    }
    
    // close connection
    close( nSocketID );

    return 0;
}

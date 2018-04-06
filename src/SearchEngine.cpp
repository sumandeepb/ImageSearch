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

#ifdef WIN32
#include <direct.h>
#else
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif
#include <utility>
#include "Common.h"
#include "SearchEngine.h"

using namespace std;
using namespace cv;

// constructor
CSearchEngine::CSearchEngine()
{
}

// destructor
CSearchEngine::~CSearchEngine()
{
	m_vecImageData.clear();
	m_cVocabTree.Clear();
}

// set image DB path folder and name in the DB folder
int CSearchEngine::CreateDB( const std::string &strPath, const std::string &strName )
{
	// clear memory before building a new database
	ClearDB();

	// set path of image database
	m_strDBPath = strPath;
#ifdef _DEBUG
	LogData( "Set DB Path: %s\n", m_strDBPath.c_str() );
#endif

	// set database name
	m_strDBName = strName;
#ifdef _DEBUG
	LogData( "Creating folder: %s...", m_strDBName.c_str() );
#endif
#ifdef WIN32
	if( -1 == _mkdir( m_strDBName.c_str() ) )
#else
    if( -1 == mkdir( m_strDBName.c_str(), 0775 ) )
#endif
	{
		cerr << "Failed mkdir command." << strPath << endl;
		cerr << "errno = " << errno << endl;
		if( EEXIST != errno )
		{
			// failed to make database folder
			cerr << "Failed to make database folder." << endl;
			return -1;
		}
	}
#ifdef _DEBUG
	LogData( "success\n" );
#endif

	string	strTemp;
	strTemp = m_strDBPath + "/" + TEMP_FOLDER;
#ifdef _DEBUG
	LogData( "Creating folder: %s...", strTemp.c_str() );
#endif
#ifdef WIN32
	if( -1 == _mkdir( m_strDBName.c_str() ) )
#else
    if( -1 == mkdir( strTemp.c_str(), 0775 ) )
#endif
	{
		cerr << "Failed mkdir command." << strTemp << endl;
		cerr << "errno = " << errno << endl;
		if( EEXIST != errno )
		{
			// failed to make temporary folder
			cerr << "Failed to make temporary folder." << endl;
			return -1;
		}
	}
#ifdef _DEBUG
	LogData( "success\n" );
#endif

	strTemp = m_strDBPath + "/" + IMAGE_FOLDER;
#ifdef _DEBUG
	LogData( "Creating folder: %s...", strTemp.c_str() );
#endif
#ifdef WIN32
	if( -1 == _mkdir( m_strDBName.c_str() ) )
#else
    if( -1 == mkdir( strTemp.c_str(), 0775 ) )
#endif
	{
		cerr << "Failed mkdir command." << strTemp << endl;
		cerr << "errno = " << errno << endl;
		if( EEXIST != errno )
		{
			// failed to make image folder
			cerr << "Failed to make image folder." << endl;
			return -1;
		}
	}
#ifdef _DEBUG
	LogData( "success\n" );
#endif

	strTemp = m_strDBPath + "/" + DESCR_FOLDER;
#ifdef _DEBUG
	LogData( "Creating folder: %s...", strTemp.c_str() );
#endif
#ifdef WIN32
	if( -1 == _mkdir( m_strDBName.c_str() ) )
#else
    if( -1 == mkdir( strTemp.c_str(), 0775 ) )
#endif
	{
		cerr << "Failed mkdir command." << strTemp << endl;
		cerr << "errno = " << errno << endl;
		if( EEXIST != errno )
		{
			// failed to make descriptor folder
			cerr << "Failed to make descriptor folder." << endl;
			return -1;
		}
	}
#ifdef _DEBUG
	LogData( "success\n" );
#endif

	return 0;
}

int CSearchEngine::LoadDB( const std::string &strPath, const std::string &strName,
	bool fLoadFullImageRecord )
{
	ClearDB();

	m_strDBPath = strPath;
	m_strDBName = strName;

	if( 0 != LoadImageDB( fLoadFullImageRecord ) )
	{
		cerr << "Failed to load image database." << endl;
		return -1;
	}

	if( 0 != LoadVocabTree() )
	{
		cerr << "Failed to load vocab tree." << endl;
		return -1;
	}

#if HIST_SEARCH
	if( 0 != LoadHashTable() )
	{
		cerr << "Failed to load hash table." << endl;
		return -1;
	}
#endif

	return 0;
}

// clear the image database data structure
void CSearchEngine::ClearDB()
{
	m_strDBPath = "";
	m_strDBName = "";

	ClearImageDB();
	ClearVocabTree();
#if HIST_SEARCH
	ClearHashTable();
#endif
}

// add single image file to database data structure
int CSearchEngine::AddFile( const std::string &strInputFilePath,
	const std::string &strImageName,
	bool fComputeHash )
{
#ifdef _DEBUG
	LogData( "Adding file: %s\n", strImageName.c_str() );
#endif

	// create new image data record
	CImageData*	pImageData = new CImageData( m_strDBPath, strImageName );

	// load image frame data onto the image record
	Mat matTempFrame = imread( strInputFilePath );
	pImageData->SetImageFrame( matTempFrame );
	if( !pImageData->IsImageValid() )
	{
#ifdef _DEBUG
		LogData( "Failed to load image file\n" );
#endif
		return -1;
	}

#ifdef _DEBUG
	LogData( "Computing Descriptors..." );
#endif
	// detect keypoints and compute descriptors
	int error = pImageData->ComputeDescriptors();
	if( 0 != error )
	{
#ifdef _DEBUG
		LogData( "Failed to compute descriptors\n" );
#endif
		return error;
	}
#ifdef _DEBUG
	LogData( "done\n" );
#endif

	// if option to compute and save hash enabled
	if( fComputeHash )
	{
		// check whether vocab tree already exists and propper image hash records exist
		if( !m_cVocabTree.IsEmpty() && m_vecImageData.size() == m_vecHashMap.size() )
		{
			// create hash for the new image record
			CImageHash* pImageHash = new CImageHash();
			pImageHash->Compute( pImageData->GetDescriptors(), m_cVocabTree );
			m_vecHashMap.push_back( pImageHash );
		}
		else
		{
#ifdef _DEBUG
				LogData( "Need to pre-create vocabulary tree and build hash table\n" );
#endif			
				return -1;
		}
	}

	// add image data record to record list
	m_vecImageData.push_back( pImageData );

	return 0;
}

// add list of images to the database data structure
int CSearchEngine::AddFileList( const std::vector<std::string> &vecInputFilePaths, const std::vector<std::string> &vecImageNames )
{
	// add image records one by one
	for( unsigned int i = 0; i < vecImageNames.size(); i++ )
	{
		cout << "Adding file: " << vecImageNames[i] << endl;
		// add image record
		int error = AddFile( vecInputFilePaths[i], vecImageNames[i] );
		if( 0 != error )
		{
			return error;
		}
	}

	return 0;
}

// save image data records
int CSearchEngine::SaveImageDB ()
{
	vector<String>	vecImageNames;
	// save image records one by one
	for( list<CImageData*>::iterator it = m_vecImageData.begin(); it != m_vecImageData.end(); it++ )
	{
		vecImageNames.push_back( (*it)->GetImageName() );
#ifdef _DEBUG
		LogData( "Saving record: %s...", vecImageNames.back().c_str() );
#endif
		// save image record
		int error = (*it)->SaveImageRecord();
		if( 0 != error )
		{
			return error;
		}
#ifdef _DEBUG
		LogData( "success\n" );
#endif
	}

#ifdef _DEBUG
	LogData( "Saving Image DB File...\n" );
#endif
	// save record list file
	FileStorage fs( m_strDBPath + "/" + m_strDBName + MAIN_FILE + FILE_FORMAT, FileStorage::WRITE );
	if( !fs.isOpened() )
	{
		return -1;
	}
	write( fs, "images", vecImageNames );
	fs.release();
#ifdef _DEBUG
	LogData( "success\n" );
#endif

	return 0;
}

// load image data records
int CSearchEngine::LoadImageDB( bool fLoadFullImageRecord )
{
	ClearImageDB();

#ifdef _DEBUG
	LogData( "Loading Image DB File...\n" );
#endif
	// load record list file
	vector<String>	vecImageNames;
	FileStorage fs( m_strDBPath + "/" + m_strDBName + MAIN_FILE + FILE_FORMAT, FileStorage::READ );
	if( !fs.isOpened() )
	{
		return -1;
	}
	FileNode fs_imgnode = fs["images"];
	read( fs_imgnode, vecImageNames );
	fs.release();

	// load image records one by one
	for( vector<String>::iterator it = vecImageNames.begin(); it != vecImageNames.end(); it++ )
	{
#ifdef _DEBUG
		LogData( "Loading record: %s...", (*it).c_str() );
#endif
		CImageData*	pImageData = new CImageData( m_strDBPath, *it );

		// load image frame, keypoint and descriptors only if requested
		if( fLoadFullImageRecord )
		{
			// load image record
			int error = pImageData->LoadImageRecord();
			if( 0 != error )
			{
				return error;
			}
		}

		m_vecImageData.push_back( pImageData );
#ifdef _DEBUG
		LogData( "success\n" );
#endif
	}

	return 0;
}

// clear image database (from memory)
void CSearchEngine::ClearImageDB()
{
	for( list<CImageData*>::iterator it = m_vecImageData.begin(); it != m_vecImageData.end(); it++ )
	{
		delete *it;
	}
	m_vecImageData.clear();
}

// build the vocabulary tree
int CSearchEngine::BuildVocabTree( const int nNumClusters, const int nTreeLevels )
{
	ClearVocabTree();
#ifdef _DEBUG
	LogData( "Building vocabulary tree...\n" );
#endif	
	return m_cVocabTree.BuildTree( m_vecImageData, nNumClusters, nTreeLevels );
}

// save vocabulary tree
int CSearchEngine::SaveVocabTree() const
{
#ifdef _DEBUG
	LogData( "Saving vocabulary tree...\n" );
#endif	
	return m_cVocabTree.SaveTree( m_strDBPath + "/" + m_strDBName + VOCAB_FILE + FILE_FORMAT );
}

// load vocabulary tree
int CSearchEngine::LoadVocabTree()
{
	ClearVocabTree();
#ifdef _DEBUG
	LogData( "Loading vocabulary tree...\n" );
#endif	
	return m_cVocabTree.LoadTree( m_strDBPath + "/" + m_strDBName + VOCAB_FILE + FILE_FORMAT );
}

// clear vocabulary tree
void CSearchEngine::ClearVocabTree()
{
	m_cVocabTree.Clear();
}

#if HIST_SEARCH
// build word histogram for each image in DB
int CSearchEngine::BuildHashTable()
{
#ifdef _DEBUG
	LogData( "Building hash table..." );
#endif	

	// clean up hash table
	ClearHashTable();

	// create a new empty hash map
	m_vecHashMap.resize( m_vecImageData.size() );
	for( list<CImageHash*>::iterator it = m_vecHashMap.begin(); it != m_vecHashMap.end(); it++ )
	{
		*it = new CImageHash;
	}

	// compute image hash for all image data records
	list<CImageHash*>::iterator it_hash = m_vecHashMap.begin();
	for( list<CImageData*>::iterator it = m_vecImageData.begin(); it != m_vecImageData.end(); it++, it_hash++ )
	{
		// compute hash map for each entry
		(*it_hash)->Compute( (*it)->GetDescriptors(), m_cVocabTree );
	}

#ifdef _DEBUG
	LogData( "success\n" );
#endif

	return 0;
}

// save hash table
int CSearchEngine::SaveHashTable() const
{
	const std::string &strFileName = m_strDBPath + "/" + m_strDBName + HASH_FILE + FILE_FORMAT;
	// open file storage for writing
	FileStorage fs( strFileName, FileStorage::WRITE );
	if( !fs.isOpened() )
	{
		return -1;
	}

	int error = 0;
	fs << "hashtable" << "[";
	// save each image hash
	for( list<CImageHash*>::const_iterator it = m_vecHashMap.begin(); it != m_vecHashMap.end(); it++ )
	{
		error = (*it)->SaveImageHash( fs );
	}
	fs << "]";
	fs.release();

	return error;
}

// load hash table
int CSearchEngine::LoadHashTable()
{
	ClearHashTable();

	const std::string &strFileName = m_strDBPath + "/" + m_strDBName + HASH_FILE + FILE_FORMAT;
	// open file storage for reading
	FileStorage fs( strFileName, FileStorage::READ );
	if( !fs.isOpened() )
	{
		return -1;
	}

	int error = 0;
	FileNode fn = fs["hashtable"];
	// load each image hash
	for( FileNodeIterator it = fn.begin(); it != fn.end(); it++ )
	{
		CImageHash* pImageHash = new CImageHash;
        FileNode fn_entry = *it;
		error = pImageHash->LoadImageHash( fn_entry );
		m_vecHashMap.push_back( pImageHash );
	}
	fs.release();

	return error;
}

// clear hash table
void CSearchEngine::ClearHashTable()
{
	for( list<CImageHash*>::iterator it = m_vecHashMap.begin(); it != m_vecHashMap.end(); it++ )
	{
		delete *it;
	}
	m_vecHashMap.clear();
}
#endif

// search for a query image in database
int CSearchEngine::SearchDB( const std::string &strQueryImgFile,
    std::vector< std::string > &vecBestMatches ) const
{
#ifdef _DEBUG
	LogData( "Searching...\n" );
#endif	

	// load query image frame
	Mat	matQueryImage;
	matQueryImage = imread( strQueryImgFile );
	if( !matQueryImage.data )
    {
        return -1;
    }
    
	// create query image record
	CImageData	cQueryImage( string(""), string("SearchQuery") );
	cQueryImage.SetImageFrame( matQueryImage );
	cQueryImage.ComputeDescriptors();

#if HIST_SEARCH
	// compute word histogram for query descriptors
	CImageHash	cQueryHashMap;
	cQueryHashMap.Compute( cQueryImage.GetDescriptors(), m_cVocabTree );

	// map of top matches
	map< double, const CImageData*, greater<double> > mapBestMatches;

	// best match index
	list<CImageData*>::const_iterator it_image = m_vecImageData.begin();
	// compute best matching hash
	for( list<CImageHash*>::const_iterator it = m_vecHashMap.begin(); it != m_vecHashMap.end(); it++, it_image++ )
	{
		// compare query hash map with all hashes in the database
		double dMatchScore = (*it)->Compare( cQueryHashMap );
		mapBestMatches.insert( pair<double, const CImageData*>( dMatchScore, *it_image ) );
	}
#elif SCORE_SEARCH
	// score map for matching database images
	map<int, double> mapScore;

	// for all descriptors in query image
	int nNumDescriptors = matQueryDescriptors.rows;
	for( int iRow = 0; iRow < nNumDescriptors; iRow++ )
	{
		// search for best matching leaf node
		const CVocabTreeNode* pLeafNode = m_cVocabTree.SearchTree( matQueryDescriptors.row( iRow ) );

		// retrieve leaf node information
		double dNodeWt = pLeafNode->GetNodeWeight();
		const vector<int>& vecInvertedList = pLeafNode->GetInvertedList();
		const vector<int>& vecTermFreqList = pLeafNode->GetTermFreqList();

		// for all entries in the leaf node inverted list
		for( unsigned int iInvFile = 0; iInvFile < vecInvertedList.size(); iInvFile++ )
		{
			// get inverted file index
			int nFileIndex = vecInvertedList[iInvFile];

			// compute TF-IDF weight
			double dWeightedWordFrequency = double(vecTermFreqList[iInvFile]) * dNodeWt;

			// check for existing entry for file index in the score map
			map<int, double>::iterator it = mapScore.find( nFileIndex );

			// add entry to score map
			if ( mapScore.end() ==  it )
			{
				// new entry
				mapScore.insert( pair<int, double>( nFileIndex, dWeightedWordFrequency ) );
			}
			else
			{
				// existing entry
				it->second += dWeightedWordFrequency;
			}
		}
	}

	// search for file index with best score
	int i = 0;
	double dBestScore = 0.0;
	for( map<int, double>::const_iterator it = mapScore.begin(); it != mapScore.end(); it++, i++ )
	{
		if( it->second > dBestScore )
		{
			dBestScore = it->second;
			iBestMatch = i;
		}
	}
#endif

	// select top matches for spatial consistency re-ranking
    vecBestMatches.clear();
	map< double, const CImageData*, greater<double> >::const_iterator it_bestmatch = mapBestMatches.begin();
	for( int iBestMatch = 0; iBestMatch < NUM_TOP_MATCHES && it_bestmatch != mapBestMatches.end(); iBestMatch++, it_bestmatch++ )
	{
		cout << iBestMatch + 1 << ". "
			<< it_bestmatch->second->GetImageName() << " score = "
			<< it_bestmatch->first << endl;
        
        //pair<double, const string&> match( it_bestmatch->first, it_bestmatch->second->GetImageName() );
        vecBestMatches.push_back( it_bestmatch->second->GetImageName() );
        
		//stringstream strBuffer;
		//strBuffer << "result" << iBestMatch + 1 << ".jpg";
		
		//it_bestmatch->second->LoadImageRecord();
		//const Mat matResult = it_bestmatch->second->GetImageFrame();
		//imwrite( strBuffer.str(), matResult );
		//strBuffer.str( string() );
	}

	//return (mapBestMatches.begin()->second)->GetImageName();
    return 0;
}

#if 0
	// build list of all leaf nodes
	list<const CVocabTreeNode*>	lstLeafList;
	m_cVocabTree.BuildLeafList( lstLeafList );

	// retrieve inverted files to populate word 
	unsigned int nBinIdx = 0;
	// loop over all leaf nodes
	for( list<const CVocabTreeNode*>::const_iterator it = lstLeafList.begin(); it != lstLeafList.end(); it++, nBinIdx++ )
	{
		// retrieve leaf node information
		double dNodeWt = (*it)->GetNodeWeight();
		const vector<int>& vecInvertedList = (*it)->GetInvertedList();
		const vector<int>& vecTermFreqList = (*it)->GetTermFreqList();
		// for all entries in the leaf node inverted list
  		for( unsigned int iInvFile = 0; iInvFile < vecInvertedList.size(); iInvFile++ )
		{
			// get reference to image record from inverted file index
			const CImageData* pImageRecord = m_vecImageData[vecInvertedList[iInvFile]];
			CImageHash* pHashMap = m_vecHashMap[vecInvertedList[iInvFile]];
			// compute TF-IDF weight
			double dWeightedWordFrequency = dNodeWt * double(vecTermFreqList[iInvFile]) / double(pImageRecord->GetDescriptors().rows);
			// Add entry to sparse histogram
			pHashMap->AddEntry( nBinIdx, dWeightedWordFrequency );
		}
	}

	// compute magnitude for each hash vector
	for( vector<CImageHash*>::iterator it = m_vecHashMap.begin(); it != m_vecHashMap.end(); it++ )
	{
		(*it)->ComputeMagnitude();
	}
#endif

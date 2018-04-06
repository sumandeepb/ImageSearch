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

#include "ImageDB.h"
#include "VocabTree.h"
#include "ImageHash.h"

// alternative search algorithms (currently only HIST_SEARCH is implemented)
#define HIST_SEARCH 1
//#define SCORE_SEARCH 1

// core class for image search engine
class CSearchEngine
{
protected:
	std::string					m_strDBPath;			// path of image database folder
	std::string					m_strDBName;			// name of image database file
	std::list<CImageData*>		m_vecImageData;			// dynamic array of image data
	CVocabTree					m_cVocabTree;			// vocabulary tree (bag of features)
#if HIST_SEARCH
	std::list<CImageHash*>		m_vecHashMap;			// dynamic array of word histograms as image hash
#endif

	void ClearImageDB();								// clear image database (from memory)
	void ClearVocabTree();								// clear vocabulary tree
#if HIST_SEARCH
	void ClearHashTable();								// clear hash table
#endif

public:
	CSearchEngine();									// constructor
	~CSearchEngine();									// destructor

	int CreateDB( const std::string &strPath,
		const std::string &strName );					// set image DB path folder and name in the DB folder
	void ClearDB();										// clear the image database data structure

	int AddFile( const std::string &strInputFilePath,
		const std::string &strImageName,
		bool fComputeHash = false );					// add single image file to database data structure
	int AddFileList( const std::vector<std::string> &vecInputFilePaths, // add list of images to the database data structure
		const std::vector<std::string> &vecImageNames );// used for adding images in bulk during training phase
														
	int SaveImageDB();									// save image data records
	int LoadImageDB( bool fLoadFullImageRecord = true );// load image data records

	int BuildVocabTree( const int nNumClusters = 10,
		const int nTreeLevels = 6 );					// build the vocabulary tree
	int SaveVocabTree() const;							// save vocabulary tree
	int LoadVocabTree();								// load vocabulary tree

#if HIST_SEARCH
	int BuildHashTable();								// build word histogram for each image in DB
	int SaveHashTable() const;							// save hash table
	int LoadHashTable();								// load hash table
#endif

	int LoadDB( const std::string &strPath,
		const std::string &strName,
		bool fLoadFullImageRecord = true );				// load search database from disk to memory
	int SearchDB( const std::string &strQueryImgFile,
        std::vector< std::string > &vecBestMatches ) const; // search for a query image in database
};

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

#include "Common.h"
#include "ImageHash.h"

using namespace std;
using namespace cv;

// constructor
CImageHash::CImageHash()
{
	m_dMagnitude = 0.0;
}

// destructor
CImageHash::~CImageHash()
{
	Clear();
}

// compute word histogram from set of image descriptors
void CImageHash::Compute ( const cv::Mat &matQueryDescriptors, const CVocabTree &cVocabTree )
{
	// clear word histogram before computing a new one
	m_mapWordHist.clear();

	int nNumDescriptors = matQueryDescriptors.rows;
	// for descriotors of all keypoints
	for( int iRow = 0; iRow < nNumDescriptors; iRow++ )
	{
		// search for the closest leaf node
		const CVocabTreeNode* pLeafNode = cVocabTree.SearchTree( matQueryDescriptors.row( iRow ) );
		double dNodeWt = pLeafNode->GetNodeWeight();
		int iLeafNode = pLeafNode->GetLeafIndex();

		// check for existing entry for leaf index in the histogram
		map<int, double>::iterator it = m_mapWordHist.find( iLeafNode );
		if ( m_mapWordHist.end() ==  it )
		{
			// new entry (computes and adds the inverse document frequency)
			m_mapWordHist.insert( pair<int, double>( iLeafNode, dNodeWt / double(nNumDescriptors) ) );
		}
		else
		{
			// existing entry (keeps increasing the term frequency)
			it->second += dNodeWt / double(nNumDescriptors);
		}
	}

	// compute the magnitude of histogram for the purpose of normalization
	m_dMagnitude = 0.0;
	for( map<int, double>::iterator it = m_mapWordHist.begin(); it != m_mapWordHist.end(); it++ )
	{
		m_dMagnitude += it->second * it->second;
	}

	m_dMagnitude = sqrt( m_dMagnitude );
}

// add entries to the word histogram
//void CImageHash::AddEntry( int nBinIdx, double dWordFrequency )
//{
//	m_mapWordHist.insert( pair<int, double>( nBinIdx, dWordFrequency ) );
//}
//
// compute hash magnitude for normalization
//void CImageHash::ComputeMagnitude()
//{
//	m_dMagnitude = 0.0;
//	for( map<int, double>::iterator it = m_mapWordHist.begin(); it != m_mapWordHist.end(); it++ )
//	{
//		m_dMagnitude += it->second * it->second;
//	}
//
//	m_dMagnitude = sqrt( m_dMagnitude );
//}

// clear histogram
void CImageHash::Clear()
{
	m_dMagnitude = 0.0;
	m_mapWordHist.clear();
}

// save hash map to XML/YAML file
int CImageHash::SaveImageHash( cv::FileStorage &fs ) const
{
	fs << "{";
	fs << "magnitude" << m_dMagnitude;
	fs << "wordhist" << "[";
	for( map<int, double>::const_iterator it = m_mapWordHist.begin(); it != m_mapWordHist.end(); it++ )
	{
		fs << "{";
		fs << "bin" << it->first;
		fs << "freq" << it->second;
		fs << "}";
	}
	fs << "]";
	fs << "}";

	return 0;
}

// load hash map from XML/YAML file
int CImageHash::LoadImageHash( cv::FileNode &fn )
{
	Clear();

	fn["magnitude"] >> m_dMagnitude;
	FileNode fn_wordhist = fn["wordhist"];
	for( FileNodeIterator it = fn_wordhist.begin(); it != fn_wordhist.end(); it++ )
	{
		int nBinIdx;
		double dTermFreq;
		(*it)["bin"] >> nBinIdx;
		(*it)["freq"] >> dTermFreq;
		m_mapWordHist.insert( pair<int, double>( nBinIdx, dTermFreq ) );
	}

	return 0;
}

// compare hash with query
double CImageHash::Compare( const CImageHash &cQueryHistogram ) const
{
	// initialize the iterators
	map<int, double>::const_iterator 
		itTarget = this->m_mapWordHist.begin(),
		itTargetEnd = this->m_mapWordHist.end(),
		itQuery = cQueryHistogram.m_mapWordHist.begin(),
		itQueryEnd = cQueryHistogram.m_mapWordHist.end();

	double dScore = 0.0;
	// run a double chain through the target and query histograms
	while( itTarget != itTargetEnd && itQuery != itQueryEnd )
	{
		if( itTarget->first < itQuery->first ) // target index is less than query index
		{
			itTarget++;	// skip target bin
		}
		else if( itTarget->first > itQuery->first ) // query index is less than target index
		{
			itQuery++; // skip query bin
		}
		else // target and query index match
		{
			// accumulate score
			dScore += itTarget->second * itQuery->second;
			// move both bins forward
			itTarget++; 
			itQuery++;
		}
	}

	// normalize the score
	dScore /= this->m_dMagnitude * cQueryHistogram.m_dMagnitude;

	return dScore;
}

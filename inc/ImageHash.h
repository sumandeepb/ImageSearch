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
#include <map>
#include <opencv2/opencv.hpp>
#include "VocabTree.h"

class CImageHash
{
protected:
	double					m_dMagnitude;						// magnitude of vocab vector
	std::map<int, double>	m_mapWordHist;						// sparse histogram of visual words ( key = binIndex, data = TF-IDF score )

public:
	CImageHash();												// constructor
	~CImageHash();												// destructor

	void Compute( const cv::Mat &matQueryDescriptors,
		const CVocabTree &cVocabTree );							// compute word histogram from set of image descriptors
	//void AddEntry( int nBinIdx, double dWordFrequency );		// add entries to the word histogram
	//void ComputeMagnitude();									// compute hash magnitude for normalization
	void Clear();												// clear histogram

	int SaveImageHash( cv::FileStorage &fs ) const;				// save hash map to XML/YAML file
	int LoadImageHash( cv::FileNode &fn );						// load hash map from XML/YAML file

	double Compare( const CImageHash &cQueryHistogram ) const;	// compare hash with query
};

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

#include <vector>
#include <list>
#include <opencv2/opencv.hpp>
#include "ImageDB.h"

// tree node class
class CVocabTreeNode
{
protected:
	CVocabTreeNode*					m_pParentNode;		// pointer to root node
	std::vector<CVocabTreeNode*>	m_vecChileNodes;	// pointers to children
	int								m_nLevelId;			// level ID for node (root is zero)

	cv::Mat							m_matNodeDescriptor;// node data: single feature descriptor
	int								m_nLeafIndex;		// unique index for each leaf node
	double							m_dNodeWeight;		// node weight
	std::vector<int>				m_vecInvIdxList;	// inverted list of image index (only at leaf nodes)
	std::vector<int>				m_vecTermFrequency;	// frequency of each image index

public:
	CVocabTreeNode();									// constructor
	~CVocabTreeNode();									// destructor

	bool IsRoot() const;								// is the node the root node of the vocab tree
	bool IsLeaf() const;								// is the node a leaf node of the vocab tree
	int BuildSubTree( const cv::Mat &matDescriptors, 
		const std::vector<int> &vecDescImgIdx,
		const int nNumImages, const int nNumClusters,
		const int nTreeLevels, const int nMAXITER );	// recursively build the subtree from this node
	int SaveSubTree( cv::FileStorage &fs ) const;		// recursively save sub tree to XML/YAML file
	int LoadSubTree( cv::FileNode &fn );				// recursively retrieve sub tree from XML/YAML file

	const cv::Mat& GetNodeDescriptor() const;			// get cluster center for node
	int GetLeafIndex() const;							// get leaf index (-1 for non-leaf nodes)
	double GetNodeWeight() const;						// get IDF( inverse document frequency) node weight
	const std::vector<int> &GetInvertedList() const;	// get reference to inverted file index list
	const std::vector<int> &GetTermFreqList() const;	// get reference to term frequency list

	int BuildLeafList( std::list<const CVocabTreeNode*> &lstLeafList ) const;	// build a list of leaf node pointers
	const CVocabTreeNode* SearchSubTree( const cv::Mat &matQueryDescr ) const;	// recursively search for the closest leaf node to the query descriptor
};

// hierarchical k-means tree class
class CVocabTree
{
protected:
	int								m_nNumClusters;		// number of clusters per node
	int								m_nTreeLevels;		// number of levels
	CVocabTreeNode*					m_pRootNode;		// pointer to root node

public:
	CVocabTree();													// constructor
	~CVocabTree();													// destructor

	bool IsEmpty() const;											// check whether tree is emptry or not
	int BuildTree( const std::list<CImageData*> &vecImageData,
		const int nNumClusters = 10, const int nTreeLevels = 6,
		const int nMAXITER = 100 );									// build vocabulary tree from linked list of image data
	int SaveTree( const std::string &strFileName ) const;			// save vocab tree to file
	int LoadTree( const std::string &strFileName );					// load vocab tree from file
	void Clear();													// clear vocabulary tree

	int BuildLeafList( std::list<const CVocabTreeNode*> &lstLeafList ) const;	// build a list of leaf node pointers
	const CVocabTreeNode* SearchTree( const cv::Mat &matQueryDescr ) const;		// returns the closest leaf node to the query descriptor
};

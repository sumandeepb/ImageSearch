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
#include "VocabTree.h"

using namespace std;
using namespace cv;

// constructor
CVocabTreeNode::CVocabTreeNode()
{
	m_pParentNode = NULL;
	m_nLevelId = 0;
	m_nLeafIndex = -1;
	m_dNodeWeight = 0.0;
}

// destructor
CVocabTreeNode::~CVocabTreeNode()
{
	for( vector<CVocabTreeNode*>::iterator it = m_vecChileNodes.begin(); it != m_vecChileNodes.end(); it++ )
	{
		delete *it;
	}
	m_vecChileNodes.clear();

	m_pParentNode = NULL;
	m_nLevelId = 0;
	m_matNodeDescriptor = Mat();
	m_nLeafIndex = -1;
	m_dNodeWeight = 0.0;
	m_vecInvIdxList.clear();
	m_vecTermFrequency.clear();
}

// is the node the root node of the vocab tree
bool CVocabTreeNode::IsRoot() const
{
	return( NULL == m_pParentNode );
}

// is the node a leaf node of the vocab tree
bool CVocabTreeNode::IsLeaf() const
{
	return( 0 == m_vecChileNodes.size() /* || m_nLeafIndex >= 0*/ );
}

// recursively build the subtree from this node
int CVocabTreeNode::BuildSubTree( const cv::Mat &matDescriptors,
	const std::vector<int> &vecDescImgIdx,
	const int nNumImages, const int nNumClusters,
	const int nTreeLevels, const int nMAXITER )
{
	// static leaf counter
	static int nLeafCounter = -1;

	// reinit leaf counter when beginning to build a tree
	if( IsRoot() )
	{
		nLeafCounter = 0;
	}

	// compute node descriptor (mean of all vectors)
	reduce( matDescriptors, m_matNodeDescriptor, 0, CV_REDUCE_AVG );

	// compute node weight
	int nNumUniqueImageIdx = 1;
	for( unsigned int i = 1; i < vecDescImgIdx.size(); i++ )
	{
		if( vecDescImgIdx[i - 1] != vecDescImgIdx[i] )
		{
			nNumUniqueImageIdx++;
		}
	}
	m_dNodeWeight = log( double(nNumImages) / double(nNumUniqueImageIdx) );

	// return if node level has reached max levels or less points than clusters
	if( matDescriptors.rows < nNumClusters || m_nLevelId >= nTreeLevels )
	{
		// set leaf index
		m_nLeafIndex = nLeafCounter;

		// increment leaf counter
		nLeafCounter++;

#if 0
		// compute term frequency and set inverted list
		int cTerms = 0;
		int iTermFreq = 1;
		m_vecTermFrequency.resize( nNumUniqueImageIdx );
		m_vecInvIdxList.resize( nNumUniqueImageIdx );
		m_vecInvIdxList[0] = vecDescImgIdx[0];
		for( unsigned int i = 1; i < vecDescImgIdx.size(); i++ )
		{
			if( vecDescImgIdx[i] != vecDescImgIdx[i - 1] )
			{
				m_vecTermFrequency[cTerms] = iTermFreq;
				cTerms++;
				m_vecInvIdxList[cTerms] = vecDescImgIdx[i];
				iTermFreq = 1;
			}
			else
			{
				iTermFreq++;
			}
		}
		m_vecTermFrequency[cTerms] = iTermFreq;
#endif

		return 0;
	}

	// arrange the descriptors into further K clusters
	Mat matLabels;
	kmeans( matDescriptors, nNumClusters, matLabels, TermCriteria( CV_TERMCRIT_ITER, nMAXITER, 1.0 ), 5, KMEANS_PP_CENTERS );

	// divide descriptors into K sub-lists
	vector<Mat> vecClusterDescr( nNumClusters );
	vector< vector<int> > vecvecDescImgIdx( nNumClusters );
	for( int i = 0; i < matDescriptors.rows; i++ )
	{
		if( vecClusterDescr[ matLabels.at<int>(i) ].rows > 0 )
		{
			vecClusterDescr[ matLabels.at<int>(i) ].push_back( matDescriptors.row( i ) );
		}
		else
		{
			vecClusterDescr[ matLabels.at<int>(i) ] = matDescriptors.row( i ).clone();
		}
		vecvecDescImgIdx[ matLabels.at<int>(i) ].push_back( vecDescImgIdx[i] );
	}

	int error = 0;
	m_vecChileNodes.resize( nNumClusters );
	// for each of K clusters
	for( int k = 0; k < nNumClusters; k++ )
	{ 
		m_vecChileNodes[k] = new CVocabTreeNode;
		m_vecChileNodes[k]->m_pParentNode = this;
		m_vecChileNodes[k]->m_nLevelId = this->m_nLevelId + 1;

		// build sub-tree
		error = m_vecChileNodes[k]->BuildSubTree( vecClusterDescr[k], vecvecDescImgIdx[k],
			nNumImages, nNumClusters, nTreeLevels, nMAXITER );
	}

	return error;
}

// recursively save sub tree to XML/YAML file
int CVocabTreeNode::SaveSubTree( cv::FileStorage &fs ) const
{
	int error = 0;

	fs << "{";
	fs << "level" << m_nLevelId;
	fs << "descriptor" << m_matNodeDescriptor;
	fs << "leafindex" << m_nLeafIndex;
	fs << "nodeweight" << m_dNodeWeight;
	fs << "invlist" << m_vecInvIdxList;
	fs << "termfreq" << m_vecTermFrequency;
	fs << "treenode" << "[";
	for( vector<CVocabTreeNode*>::const_iterator it = m_vecChileNodes.begin(); it != m_vecChileNodes.end(); it++ )
	{
		error = (*it)->SaveSubTree( fs );
	}
	fs << "]";
	fs << "}";

	return error;
}

// recursively retrieve sub tree from XML/YAML file
int CVocabTreeNode::LoadSubTree( cv::FileNode &fn )
{
	int error = 0;

	fn["level"] >> m_nLevelId;
	fn["descriptor"] >> m_matNodeDescriptor;
	fn["leafindex"] >> m_nLeafIndex;
	fn["nodeweight"] >> m_dNodeWeight;
	fn["invlist"] >> m_vecInvIdxList;
	fn["termfreq"] >> m_vecTermFrequency;
	FileNode fn_child = fn["treenode"];
	FileNodeIterator it = fn_child.begin(), it_end = fn_child.end();
	for( ; it != it_end; it++ )
	{
		m_vecChileNodes.push_back( new CVocabTreeNode );
		m_vecChileNodes.back()->m_pParentNode = this;
        FileNode fn_child_entry = *it;
		error = m_vecChileNodes.back()->LoadSubTree( fn_child_entry );
	}

	return error;
}

// get cluster center for node
const cv::Mat& CVocabTreeNode::GetNodeDescriptor() const
{
	return m_matNodeDescriptor;
}

// get leaf index
int CVocabTreeNode::GetLeafIndex() const
{
	return m_nLeafIndex;
}

// get inverse document frequency
double CVocabTreeNode::GetNodeWeight() const
{
	return m_dNodeWeight;
}

// get reference to inverted file index list
const std::vector<int>& CVocabTreeNode::GetInvertedList() const
{
	return m_vecInvIdxList;
}

// get reference to term frequency list
const std::vector<int>& CVocabTreeNode::GetTermFreqList() const
{
	return m_vecTermFrequency;
}

// build a list of leaf node pointers
int CVocabTreeNode::BuildLeafList( std::list<const CVocabTreeNode*> &lstLeafList ) const
{
	if( this->IsLeaf() )
	{
		lstLeafList.push_back( this );
	}
	else
	{
		for( vector<CVocabTreeNode*>::const_iterator it = m_vecChileNodes.begin(); it != m_vecChileNodes.end(); it++ )
		{
			(*it)->BuildLeafList( lstLeafList );
		}
	}

	return 0;
}

// recursively search for the closest leaf node to the query descriptor
const CVocabTreeNode* CVocabTreeNode::SearchSubTree( const cv::Mat &matQueryDescr ) const
{
	if( this->IsLeaf() )
	{
		return this;
	}
	else
	{
		// find closest cluster
		double dBestScore = INF;
		const CVocabTreeNode* pBestMatch = NULL;
		for( vector<CVocabTreeNode*>::const_iterator it = m_vecChileNodes.begin(); it != m_vecChileNodes.end(); it++ )
		{
			Mat matError = matQueryDescr - (*it)->m_matNodeDescriptor;	// difference of feature vector
			double dMatchScore = matError.dot( matError );				// inner product
			if( dMatchScore < dBestScore )
			{
				dBestScore = dMatchScore;
				pBestMatch = *it;
			}
		}

		// recursively search best matching 
		return pBestMatch->SearchSubTree( matQueryDescr );
	}
}

// constructor
CVocabTree::CVocabTree()
{
	m_pRootNode = NULL;
	m_nNumClusters = 10;
	m_nTreeLevels = 6;
}

// destructor
CVocabTree::~CVocabTree()
{
	Clear();
}

// check whether tree is emptry or not
bool CVocabTree::IsEmpty() const
{
	return ( NULL == m_pRootNode );
}

// build vocabulary tree from linked list of image data
int CVocabTree::BuildTree( const std::list<CImageData*> &vecImageData, 
	const int nNumClusters, const int nTreeLevels, const int nMAXITER )
{
	Clear();

	// return error if image DB is empty
	if( vecImageData.size() < 1 )
	{
		return -1;
	}

	// set k-means parameters
	m_nNumClusters = nNumClusters;
	m_nTreeLevels = nTreeLevels;

	// create matrix of all descriptors
	int idx = 0, prevRow, currRow;
	Mat	matDescriptors;
	vector<int> vecDescImgIdx;

	// initialize set of descriptors and index of the first image
	prevRow = matDescriptors.rows;
	list<CImageData*>::const_iterator it = vecImageData.begin();
	matDescriptors = (*it)->GetDescriptors().clone();
	currRow = matDescriptors.rows;
	for( int i = prevRow; i < currRow; i++ )
	{
		vecDescImgIdx.push_back( idx );
	}

	// add descriptors and indices from subsequent images
	for( idx++, it++; it != vecImageData.end(); idx++, it++ )
	{
		prevRow = currRow;
		matDescriptors.push_back( (*it)->GetDescriptors() );
		currRow = matDescriptors.rows;
		for( int i = prevRow; i < currRow; i++ )
		{
			vecDescImgIdx.push_back( idx );
		}
	}

	// build tree recursively from the root node
	m_pRootNode = new CVocabTreeNode;
	return m_pRootNode->BuildSubTree( matDescriptors, vecDescImgIdx,
		vecImageData.size(), m_nNumClusters, m_nTreeLevels, nMAXITER );
}

// save vocab tree to file
int CVocabTree::SaveTree( const std::string &strFileName ) const
{
	FileStorage fs( strFileName, FileStorage::WRITE );
	if( !fs.isOpened() )
	{
		return -1;
	}

	int error = 0;
	fs << "clusters" << m_nNumClusters;
	fs << "maxlevel" << m_nTreeLevels;
	fs << "treenode" << "[";
	if( NULL != m_pRootNode)
	{
		error = m_pRootNode->SaveSubTree( fs );
	}
	fs << "]";
	fs.release();

	return error;
}

// load vocab tree from file
int CVocabTree::LoadTree( const std::string &strFileName )
{
	Clear();

	FileStorage fs( strFileName, FileStorage::READ );
	if( !fs.isOpened() )
	{
		return -1;
	}

	int error = 0;
	fs["clusters"] >> m_nNumClusters;
	fs["maxlevel"] >> m_nTreeLevels;
	FileNode fn = fs["treenode"];
	FileNodeIterator it = fn.begin();
	m_pRootNode = new CVocabTreeNode;
    FileNode fn_root_entry = *it;
	error = m_pRootNode->LoadSubTree( fn_root_entry );

	fs.release();

	return error;
}

// clear vocabulary tree
void CVocabTree::Clear()
{
	if( NULL != m_pRootNode )
	{
		delete m_pRootNode;
		m_pRootNode = NULL;
	}

	m_nNumClusters = 10;
	m_nTreeLevels = 6;
}

// build a list of leaf node pointers
int CVocabTree::BuildLeafList( std::list<const CVocabTreeNode*> &lstLeafList ) const
{
	if( NULL == m_pRootNode )
	{
		return -1;
	}

	return m_pRootNode->BuildLeafList( lstLeafList );
}

// returns the closest leaf node to the query descriptor
const CVocabTreeNode* CVocabTree::SearchTree( const cv::Mat &matQueryDescr ) const
{
	if( 1 != matQueryDescr.rows && 128 != matQueryDescr.cols )
	{
		return NULL;
	}

	return m_pRootNode->SearchSubTree( matQueryDescr );
}

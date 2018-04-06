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
#include "ImageDB.h"

using namespace std;
using namespace cv;

// keypoint feature descriptor computation class
cv::SURF g_SURFDetector( 400 );
// descriptor matcher (BBF+NN) class
cv::FlannBasedMatcher g_FLANNMatcher;

// create image database record with path to the database and name of image
CImageData::CImageData( const std::string &strDBPath, const std::string &strImageName ) : m_strDBPath(strDBPath), m_strImageName(strImageName)
{
	m_fRecordSaved = true;
}

// destructor
CImageData::~CImageData()
{
}

// set image frame data
void CImageData::SetImageFrame( const cv::Mat &matImageFrame )
{
	m_fRecordSaved = false;

	// if any dimention of the input image exceeds the cap, rescale the image
	if( matImageFrame.cols > MAX_WIDTH || matImageFrame.rows > MAX_HEIGHT )
	{
		// compute resize factor without affecting aspect ratio
		double dResizeFactor = 1.0 / MAX(double(matImageFrame.cols)/double(MAX_WIDTH), double(matImageFrame.rows)/double(MAX_HEIGHT));
		resize( matImageFrame, m_matImageFrame, Size(), dResizeFactor, dResizeFactor, INTER_AREA );
	}
	else
	{
		// copy image frame as it is
		m_matImageFrame = matImageFrame;
	}
}

// checks if the image read was success
bool CImageData::IsImageValid() const
{
	return ( NULL != m_matImageFrame.data );
}

// computes keypoints and descriptors
int CImageData::ComputeDescriptors()
{
	// check valid image data before computing descriptors
	if( NULL == m_matImageFrame.data )
	{
		return -1;
	}

	m_fRecordSaved = false;
    //initModule_nonfree();
	g_SURFDetector.detect( m_matImageFrame, m_vecKeypoints );
	g_SURFDetector.compute( m_matImageFrame, m_vecKeypoints, m_matDescriptors );

#ifdef _DEBUG
	Mat	matKeyImage;
	drawKeypoints( m_matImageFrame, m_vecKeypoints, matKeyImage );
	imwrite( m_strDBPath + "/" + TEMP_FOLDER + "/" + m_strImageName + "_key.jpg", matKeyImage );
#endif

	return 0;
}

// saves image to jpg file and descriptors to xml file
int CImageData::SaveImageRecord()
{
	// avoid resaving the record
	if( m_fRecordSaved )
	{
		return 0;
	}

	// save image frame data to file
	imwrite( m_strDBPath + "/" + IMAGE_FOLDER + "/" + m_strImageName + ".jpg", m_matImageFrame );

	// save keypoint and descriptor data
	FileStorage fs( m_strDBPath + "/" + DESCR_FOLDER + "/" + m_strImageName + FILE_FORMAT, FileStorage::WRITE );
	if( !fs.isOpened() )
	{
		return -1;
	}
	write( fs, "keypoints", m_vecKeypoints );
	write( fs, "descriptors", m_matDescriptors );
	fs.release();

	m_fRecordSaved = true;

	return 0;
}

// loads image and descriptors
int CImageData::LoadImageRecord()
{
	// load image frame data from file
	m_matImageFrame = imread( m_strDBPath + "/" + IMAGE_FOLDER + "/" + m_strImageName + ".jpg" );

	// load keypoint and descriptor data
	FileStorage fs( m_strDBPath + "/" + DESCR_FOLDER + "/" + m_strImageName + FILE_FORMAT, FileStorage::READ );
	if( !fs.isOpened() )
	{
		return -1;
	}
	FileNode fn_keynode = fs["keypoints"];
	FileNode fn_descnode = fs["descriptors"];
	read( fn_keynode, m_vecKeypoints );
	read( fn_descnode, m_matDescriptors );
	fs.release();

	m_fRecordSaved = true;

	return 0;
}

// get image name
const std::string& CImageData::GetImageName( ) const
{
	return m_strImageName;
}

// get image frame data
const cv::Mat& CImageData::GetImageFrame() const
{
	return m_matImageFrame;
}

// get image keypoints
const std::vector<cv::KeyPoint>& CImageData::GetKeypoints() const
{
	return m_vecKeypoints;
}

// get keypoint descriptors
const cv::Mat& CImageData::GetDescriptors() const
{
	return m_matDescriptors;
}

// validate spatial consistency
int CImageData::ValidateGeometry( const CImageData &cQueryImage ) const
{
	// Retrieve keypoint and descriptors from query image record
	const Mat &matQueryImageFrame = cQueryImage.GetImageFrame();
	const vector<KeyPoint> &vecQueryKeypoints = cQueryImage.GetKeypoints();
	const Mat &matQueryDescriptors = cQueryImage.GetDescriptors();

	// Matching descriptor vectors using FLANN matcher
	vector<DMatch> vecMatches;
	g_FLANNMatcher.match( matQueryDescriptors, m_matDescriptors, vecMatches );

	// Quick calculation of max and min distances between keypoints
	double dMaxDist = 0; double dMinDist = 100;
	for( int i = 0; i < matQueryDescriptors.rows; i++ )
	{ 
		double dist = vecMatches[i].distance;
		if( dist < dMinDist )
			dMinDist = dist;
		if( dist > dMaxDist )
			dMaxDist = dist;
	}

	// Draw only "good" matches (i.e. whose distance is less than 3*min_dist )
	vector<DMatch> vecGoodMatches;
	for( int i = 0; i < matQueryDescriptors.rows; i++ )
	{
		if( vecMatches[i].distance < 3 * dMinDist )
		{
			vecGoodMatches.push_back( vecMatches[i] );
		}
	}

	// Localize the object
	vector<Point2f> vecQueryPoints;
	vector<Point2f> vecObjectPoints;
	for( unsigned int i = 0; i < vecGoodMatches.size(); i++ )
	{
		// Get the keypoints from the good matches
		vecQueryPoints.push_back( vecQueryKeypoints[ vecGoodMatches[i].queryIdx ].pt );
		vecObjectPoints.push_back( m_vecKeypoints[ vecGoodMatches[i].trainIdx ].pt );
	}

	// Compute planar homography to determine valid matches
	Mat matInliers;
	Mat H = findHomography( vecObjectPoints, vecQueryPoints, matInliers, CV_RANSAC );
	
	Mat img_matches;
	vector<char> vecInliers = Mat_<unsigned char>(matInliers);
	drawMatches( matQueryImageFrame, vecQueryKeypoints, m_matImageFrame, m_vecKeypoints, 
		vecGoodMatches, img_matches, Scalar::all(-1), Scalar::all(-1),
		vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS );

	imwrite( "match.jpg", img_matches );

	return 0;
}

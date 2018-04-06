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
#include <list>
#include <vector>
#include <opencv2/opencv.hpp>
#include <opencv2/nonfree/nonfree.hpp>

// keypoint feature descriptor computation class
extern cv::SURF g_SURFDetector;

// descriptor matcher (BBF+NN) class
extern cv::FlannBasedMatcher g_FLANNMatcher;

// class to store image information
class CImageData
{
protected:
	bool						m_fRecordSaved;			// whether image record is saved to disk 
	const std::string			&m_strDBPath;			// reference to DB path
	std::string					m_strImageName;			// image file name in database
	cv::Mat						m_matImageFrame;		// image frame
	std::vector<cv::KeyPoint>	m_vecKeypoints;			// keypoints
	cv::Mat						m_matDescriptors;		// keypoint descriptors

public:
	CImageData( const std::string &strDBPath,
		const std::string &strImageName );				// create image database record with path to the database and name of image
	~CImageData();										// destructor

	void SetImageFrame( const cv::Mat &matImageFrame ); // set image frame data
	bool IsImageValid() const;							// checks if the image read was success
	int ComputeDescriptors();							// computes keypoints and descriptors
	int SaveImageRecord();								// saves image to jpg file and descriptors to xml file
	int LoadImageRecord();								// loads image and descriptors

	const std::string& GetImageName() const;			// get image name
	const cv::Mat& GetImageFrame() const;				// get image frame data
	const std::vector<cv::KeyPoint>& GetKeypoints() const; // get image keypoints
	const cv::Mat& GetDescriptors() const;				// get keypoint descriptors

	int ValidateGeometry( const CImageData &cQueryImage ) const; // validate spatial consistency
};

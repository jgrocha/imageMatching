#include "GeocodeMonuments.h"

#include "Common/Log.h"

#include <iostream>
#include <dirent.h>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

GeocodeMonuments::GeocodeMonuments( Detector detID, DescriptorExtractor desID, DescriptorMatcher dmID)
{
	_fH = new FeatureHandler( detID, desID, dmID );

	_ratio = 0.85;
	_nMatchesThresh = 18;
	_showMatches = false;
}

void GeocodeMonuments::setMonument( std::string name, std::string imagePath )
{
	Image* img = processImage(imagePath);

	_monuments.push_back( std::make_pair< std::string, Image* >( name, img) );
}

void GeocodeMonuments::findMonument( std::string imagePath )
{
	Image* img = processImage(imagePath);

	bool found = false;
	std::vector< std::pair< std::string, Image*  > >::iterator it = _monuments.begin();
	for( ; !found && it != _monuments.end(); it++ )
	{
		_fH->findGoodMatches( img, it->second, _ratio );
		int matchThreshold = img->getGoodMatches( it->second ).size();

		if( matchThreshold >= _nMatchesThresh )
		{
			std::cout << "Found Monument for image '" << imagePath << "': " << it->first
					<< ". With " << matchThreshold << " feature matches."<< std::endl;

			if(_showMatches)
			{
				cv::Mat out;
				img->drawMatches( it->second, matchThreshold, MA_GD, out, cv::Scalar(255,0,0,255) );
				cv::namedWindow("Matches", CV_WINDOW_NORMAL);
				cv::imshow("Matches", out);
				cv::waitKey(0);
			}
			found = true;
		}
	}
	if( !found ) std::cout << "Was not able to find monument for image '" << imagePath << "'." << std::endl;
}

void GeocodeMonuments::testDir( std::string path )
{
	getFileList(path);

	std::set< std::string >::iterator it = _files.begin();
	for( ; it != _files.end(); it++ )
	{
		findMonument( path+"/"+*it );
	}
}

Image* GeocodeMonuments::processImage( std::string path )
{
	Image* img = new Image( path, 0.0f, 0.0f, 0.0f, 0.0f );

	_fH->detect( img );
	_fH->computeDescriptors( img );

	return img;
}

void GeocodeMonuments::setShowMatches(bool value)
{
	_showMatches = value;
}

void GeocodeMonuments::printFileNames()
{
	for( std::set< std::string >::iterator it = _files.begin(); it != _files.end(); it++ )
	{
		std::cout << (*it) << std::endl;
	}
}

void GeocodeMonuments::getFileList( std::string path )
{
	DIR *dir;
	struct dirent *ent;
	if( ( dir = opendir( path.c_str() ) ) != NULL )
	{
		while( ( ent = readdir( dir ) ) != NULL)
		{
			if( ent->d_type == DT_REG )
			{
				_files.insert( std::string( ent->d_name ) );
			}
		}
		closedir (dir);
	}
	else
	{
	  Log::exit_error("Unable to access directory " + path );
	}
}

GeocodeMonuments::~GeocodeMonuments() {}

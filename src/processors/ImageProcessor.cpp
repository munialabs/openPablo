/*
 *  ImageProcessor.cpp
 *
 *
 *  This file is part of openPablo.
 *
 *  Copyright (c) 2012- Aydin Demircioglu (aydin@openpablo.org)
 *
 *  openPablo is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openPablo is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openPablo.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ImageProcessor.hpp"

#include <Magick++.h>
#include <magick/MagickCore.h>
#include <string>
#include <QString>
#include <QDebug>
#include <QDir>


/*
 * @mainpage ImageProcessor
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file ImageProcessor.cpp
 *
 * @brief description in brief.
 *
 */

using namespace Magick;



namespace openPablo
{

    /*
     * @class ImageProcessor
     *
     * @brief Abstract class to interface the capabilities of a processor
     *
     * Abstract class..
     *
     */


    ImageProcessor::ImageProcessor()
    {
        //
    }



    ImageProcessor::~ImageProcessor()
    {
        //

    }



    void ImageProcessor::start ()
    {
 	   // Initialize ImageMagick install location for Windows
 	  InitializeMagick(NULL);

 	  // TODO: do it correctly.
 	  Magick::Image magick;
 	  if (imageBlob.length() > 0)
 	  {
 		  magick.read(imageBlob);
 	  }
 	  else
 	  {
 	 	  qDebug() << "Opening file: " << filename.toStdString().c_str();

 	 	  // FIXME: test for empty string

 	 	  //
 	 	  magick.read(filename.toStdString());
 	  }

 	  // set ICC stuff
 	  magick.renderingIntent(Magick::PerceptualIntent);
/*
 	  magick.profile("ICC", Magick::Blob(outputProfile.constData(), outputProfile.size()));

 	  const Magick::Blob  targetICC (outputProfile.constData(), outputProfile.size());
 	  magick.profile("ICC", targetICC);
 	  magick.iccColorProfile(targetICC);

//
//    try
//    {
//	BOOST_FOREACH(const ptree::value_type& child,
//                  pt.get_child("Output"))
//	{
//	  std::string outputid = child.second.get<std::string>("id");
//	  std::cout << outputid  << "\n";
//	}


  	  std::string outputPath = child.second.get<std::string>("OutputPath");
  	  std::cout << outputPath << "\n";


  	uint32_t width = child.second.get<int>("Width");
  	uint32_t height = child.second.get<int>("Height");
  	std::cout << "Scale to " << width << ", " << height << "\n";

  	std::stringstream str;
  	str << width << "x" << height;
  	std::string resizeresult;
  	str >> resizeresult;
  	magick.resize(resizeresult);

  	// modify exif data

   	Blob blob;
  	magick.magick( "JPEG" ); // Set JPEG output format
  	magick.write( &blob );

  	Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const uint8_t*) blob.data(), (long) blob.length());

  	image->readMetadata();
  	Exiv2::ExifData &exifData = image->exifData();

  	exifData["Exif.Photo.UserComment"] = "charset=\"Unicode\" An Unicode Exif comment added with Exiv2";
  	exifData["Exif.Photo.UserComment"] = "charset=\"Undefined\" An undefined Exif comment added with Exiv2";
  	exifData["Exif.Photo.UserComment"] = "Another undefined Exif comment added with Exiv2";
  	exifData["Exif.Photo.UserComment"] = "charset=Ascii An ASCII Exif comment added with Exiv2";
  	exifData["Exif.Image.Model"] = "Test 1";                     // AsciiValue
  	exifData["Exif.Image.SamplesPerPixel"] = uint16_t(162);      // UShortValue
  	exifData["Exif.Image.XResolution"] = int32_t(-2);            // LongValue
  	exifData["Exif.Image.YResolution"] = Exiv2::Rational(-2, 3); // RationalValue


  	Exiv2::IptcData &iptcData = image -> iptcData();

  	iptcData["Iptc.Application2.Headline"] = "openPablo v0.1";
  	iptcData["Iptc.Application2.Keywords"] = "Yet another keyword";
  	iptcData["Iptc.Application2.DateCreated"] = "2004-8-3";
  	iptcData["Iptc.Application2.Urgency"] = uint16_t(1);
  	iptcData["Iptc.Envelope.ModelVersion"] = 42;
  	iptcData["Iptc.Envelope.TimeSent"] = "14:41:0-05:00";
  	iptcData["Iptc.Application2.RasterizedCaption"] = "230 42 34 2 90 84 23 146";
  	iptcData["Iptc.0x0009.0x0001"] = "Who am I?";


  	image->writeMetadata();
  	Exiv2::BasicIo &myMemIo = image->io();


  	// create blob again
  // 	unsigned char *newImage = new unsigned char[myMemIo.size()];
  // 	myMemIo.open();
  // 	myMemIo.write(newImage, myMemIo.size());


  	std::cout << myMemIo.size();
  //	DataBuf myBuf;
  //	image->writeFile (myBuf,

   	Blob newBlob((const char*) myMemIo.mmap(false), myMemIo.size());
  //	magick.update (newblob.data, blob.length);
  	magick.read(newBlob);
*/
  	magick.unsharpmask(40, 5.0, 1.0, 0);
  	magick.normalize();
  	magick.equalize();
  	magick.enhance();
  	magick.gamma(1.8);

  	//magick.write(outputPath + "/" + inputFile);
/*
    // depending on file type different things should happen.
    QDir outputDir (QString::fromStdString(pt.get<std::string>("InputPath")));
    QString imageFileName = inputDir.filePath(QString::fromStdString(pt.get<std::string>("Input.InputFile")));

	std::string outputPath = child.second.get<std::string>("OutputPath");
	std::cout << outputPath << "\n";

*/
    std::string inputFile = filename.toStdString();
  	magick.write("/tmp/" + inputFile + ".jpg");

//  	}
//      }
//        catch (const std::exception& ex)
//      {
//  	cerr << "failed to magick - " << ex.what() << endl;
//      }
//
    }



    void ImageProcessor::setBLOB (unsigned char *data, uint64_t datalength)
    {
    	// create blob
		imageBlob.updateNoCopy(data, datalength );
    }
}

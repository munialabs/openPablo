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
/*  	magick.normalize();
  	magick.equalize();
  	magick.enhance();
  	magick.gamma(1.8);
*/
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






  	//
  	//    try
  	//    {
  	//	BOOST_FOREACH(const ptree::value_type& child,
  	//                  pt.get_child("Output"))
  	//	{
  	//	  std::string outputid = child.second.get<std::string>("id");
  	//	  std::cout << outputid  << "\n";
  	//	}

//  	return -1;
  	/*
  	    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(inputPath + "/" + inputFile);
  	    assert(image.get() != 0);
  	    image->readMetadata();
  	    std::cout << inputPath + "/" + inputFile;

  	    Exiv2::ExifData &exifData = image->exifData();
  	    if (exifData.empty()) {
  	        std::string error(inputPath + "/" + inputFile);
  	        error += ": No Exif data found in the file";
  	    //    throw Exiv2::Error(1, error);
  	    }
  	    else
  	    {
  	      Exiv2::ExifData::const_iterator end = exifData.end();
  	      for (Exiv2::ExifData::const_iterator i = exifData.begin(); i != end; ++i) {
  		  const char* tn = i->typeName();
  		  std::cout << std::setw(44) << std::setfill(' ') << std::left
  			    << i->key() << " "
  			    << "0x" << std::setw(4) << std::setfill('0') << std::right
  			    << std::hex << i->tag() << " "
  			    << std::setw(9) << std::setfill(' ') << std::left
  			    << (tn ? tn : "Unknown") << " "
  			    << std::dec << std::setw(3)
  			    << std::setfill(' ') << std::right
  			    << i->count() << "  "
  			    << std::dec << i->value()
  			    << "\n";
  	      }
  	    }



  	    const struct lfMount *const *mounts;
  	    const struct lfCamera *const *cameras;
  	    const struct lfLens *const *lenses;
  	    struct lfDatabase *ldb;
  	    lfError e;

  	    ldb = lf_db_new ();
  	    if (!ldb)
  	    {
  	        fprintf (stderr, "Failed to create database\n");
  	        return -1;
  	    }

  	    std::cout << "HomeDataDir: " << ldb->HomeDataDir;

  	    lf_db_load (ldb);

  	    std::cout <<  ("< --------------- < Mounts > --------------- >\n");
  	    mounts = lf_db_get_mounts (ldb);
  	    for (uint32_t i = 0; mounts [i]; i++)
  	    {
  	        std::cout << ("Mount:") << lf_mlstr_get (mounts [i]->Name);
  	        if (mounts [i]->Compat)
  	            for (uint32_t j = 0; mounts [i]->Compat [j]; j++)
  	                std::cout << ("\tCompat: \n") << mounts [i]->Compat [j];
  	    }

  	    std::cout << ("< --------------- < Cameras > --------------- >\n");
  	    cameras = lf_db_get_cameras (ldb);
  	    for (uint32_t i = 0; cameras [i]; i++)
  	    {
  	        std::cout << ("Camera: %s / %s %s%s%s\n");
  	        std::cout <<     lf_mlstr_get (cameras [i]->Maker);
  	            std::cout << lf_mlstr_get (cameras [i]->Model);
  	            std::cout << cameras [i]->Variant;
  	            std::cout << cameras [i]->Variant << lf_mlstr_get (cameras [i]->Variant);
  	            std::cout << cameras [i]->Variant;
  	        std::cout << ("\tMount: %s\n") << lf_db_mount_name (ldb, cameras [i]->Mount);
  	        std::cout << ("\tCrop factor: %g\n") << cameras [i]->CropFactor;
  	    }
  	/*
  	    g_print ("< --------------- < Lenses > --------------- >\n");
  	    lenses = lf_db_get_lenses (ldb);
  	    for (i = 0; lenses [i]; i++)
  	    {
  	        g_print ("Lens: %s / %s\n",
  	            lf_mlstr_get (lenses [i]->Maker),
  	            lf_mlstr_get (lenses [i]->Model));
  	        g_print ("\tCrop factor: %g\n", lenses [i]->CropFactor);
  	        g_print ("\tFocal: %g-%g\n", lenses [i]->MinFocal, lenses [i]->MaxFocal);
  	        g_print ("\tAperture: %g-%g\n", lenses [i]->MinAperture, lenses [i]->MaxAperture);
  	        g_print ("\tCenter: %g,%g\n", lenses [i]->CenterX, lenses [i]->CenterY);
  	        g_print ("\tCCI: %g/%g/%g\n", lenses [i]->RedCCI, lenses [i]->GreenCCI, lenses [i]->BlueCCI);
  	        if (lenses [i]->Mounts)
  	            for (j = 0; lenses [i]->Mounts [j]; j++)
  	                g_print ("\tMount: %s\n", lf_db_mount_name (ldb, lenses [i]->Mounts [j]));
  	    }
  	*/


    }



    void ImageProcessor::setBLOB (unsigned char *data, uint64_t datalength)
    {
    	// create blob
		imageBlob.updateNoCopy(data, datalength );
    }
}

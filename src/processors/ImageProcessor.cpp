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

#include "Engine.hpp"
#include "EngineFactory.hpp"

#include <Magick++.h>
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <exiv2/exiv2.hpp>
#include <magick/MagickCore.h>

#include <list>
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
using namespace std;



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



        // --- create engine

        // get engine name and ask factory to assemble it
        //QString engineName (pt.get<std::string>("Engine").c_str());
        QString engineName = "Magick";

        Engine *engine = EngineFactory::createEngine(engineName);

        engine->setSettings(pt);
        engine->setMagickImage (magick);
//	 	  engine->setLogging (...);
        engine->start();
        magick = engine->getMagickImage ();

        // cleanup
        delete engine;


        try
        {
            // for each sink
            using boost::property_tree::ptree;
            BOOST_FOREACH(const ptree::value_type& child,
                          pt.get_child("Output"))
            {
                std::string outputid = child.second.get<std::string>("id");
                std::cout << "Processing Output Sink " << outputid  << "\n";


                // -- resize image
                uint32_t width = child.second.get<int>("Width");
                uint32_t height = child.second.get<int>("Height");
                std::cout << "Scale to " << width << ", " << height << "\n";

                std::stringstream str;
                str << width << "x" << height;
                std::string resizeresult;
                str >> resizeresult;
                magick.resize(resizeresult);


                // -- apply output ICC profile

                // obtain icc parameters
                QDir profileDir (QString::fromStdString(child.second.get<std::string>("ICC.Path")));
                std::string profileName = child.second.get<std::string>("ICC.Path");
                QString profileFullName = profileDir.filePath(QString::fromStdString(profileName));


                // load file
                QFile iccfile(profileFullName);

                QByteArray outputProfile;
                if(iccfile.open(QIODevice::ReadOnly))
                {
                    outputProfile = iccfile.readAll();
                    iccfile.close();
                }
                else
                {
                    qDebug() << ("failed to load ") << profileFullName << "\n";
                    return;
                }


                //    PdfStreamedDocument document( "tests/tmp.pdf" );

                magick.profile("ICC", Magick::Blob(outputProfile.constData(), outputProfile.size()));
                const Magick::Blob  targetICC (outputProfile.constData(), outputProfile.size());
                magick.profile("ICC", targetICC);
                magick.iccColorProfile(targetICC);


                // read output format
                std::string outputFormat = child.second.get<std::string>("FileHandling.OutputFormat");

                // depending on format need some extra infos
                if (outputFormat == "JPEG")
                {
                    std::string compression = child.second.get<std::string>("FileHandling.Compression");
                }

                // depending on format need some extra infos
                if (outputFormat == "PSD")
                {
                    std::string preserveOriginalLayer = child.second.get<std::string>("FileHandling.PreserveOriginalLayer");
                }


                // apply format specifities




                // determine if user wants to have second (original) layer (..)

                magick.read(filename.toStdString());

                list<Image> layers;

                // copy original image as layer
                layers.push_back (magick);


                // -- create engine

                // get engine name and ask factory to assemble it
                //QString engineName (pt.get<std::string>("Engine").c_str());
                QString engineName = "Magick";

                Engine *engine = EngineFactory::createEngine(engineName);

                engine->setSettings(pt);
                engine->setMagickImage (magick);
                //	 	  engine->setLogging (...);
                engine->start();
                magick = engine->getMagickImage ();

                layers.push_back (magick);

                // cleanup
                delete engine;


                Image finalPSD;
                std::string inputFile = filename.toStdString();
                writeImages( layers.begin(), layers.end(), "/tmp/" + inputFile, true );
                //finalPSD.write(





                // -- apply Metadata

                // save it in the correct output format, but in memory
                Blob blob;
                magick.magick( outputFormat );
                magick.write( &blob );

                // try to apply as much metadata as possible
                Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open((const uint8_t*) blob.data(), (long) blob.length());

                image->readMetadata();
                Exiv2::ExifData &exifData = image->exifData();

                if (exifData.empty())
                {
                    // create new exif data or is exif in format not possible?
                }

                // set exif as requested by user
                exifData["Exif.Photo.UserComment"] = "charset=\"Unicode\" An Unicode Exif comment added with Exiv2";
                exifData["Exif.Image.Model"] = "Test 1";                     // AsciiValue
                exifData["Exif.Image.SamplesPerPixel"] = uint16_t(162);      // UShortValue
                exifData["Exif.Image.XResolution"] = int32_t(-2);            // LongValue
                exifData["Exif.Image.YResolution"] = Exiv2::Rational(-2, 3); // RationalValue


                // apply IPTC
                Exiv2::IptcData &iptcData = image -> iptcData();

                iptcData["Iptc.Application2.Headline"] = "openPablo v0.1";
                iptcData["Iptc.Application2.Keywords"] = "Yet another keyword";
                iptcData["Iptc.Application2.DateCreated"] = "2004-8-3";
                iptcData["Iptc.Application2.Urgency"] = uint16_t(1);


                // write data back to blob
                image->writeMetadata();
                Exiv2::BasicIo &myMemIo = image->io();
                Blob newBlob((const char*) myMemIo.mmap(false), myMemIo.size());

                //	update image with updated metadata
                magick.read(newBlob);


                // create filename
                QString outputFileName = QString::fromStdString(pt.get<std::string>("RenamePattern"));

                // cook up all the %x's
                // ...

                // fix extension, if necessary

                // create outputpath
                QDir outputDir (QString::fromStdString(child.second.get<std::string>("OutputPath")));
                QString outputFullName = outputDir.filePath(outputFileName);


                // save image
                magick.write(outputFullName.toStdString());
            }
        }
        catch (const std::exception& ex)
        {
            std::cout << "failed to magick - " << ex.what() << endl;
        }
    }



    void ImageProcessor::setBLOB (unsigned char *data, uint64_t datalength)
    {
        // create blob
        imageBlob.updateNoCopy(data, datalength );
    }
}

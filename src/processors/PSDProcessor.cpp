/*
 *  PSDProcessor.cpp
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

#include "PSDProcessor.hpp"

#include <Magick++.h>
#include <magick/MagickCore.h>
#include <list>
#include <string>
#include <QString>
#include <QDebug>
#include <QDir>


/*
 * @mainpage PSDProcessor
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file PSDProcessor.cpp
 *
 * @brief description in brief.
 *
 */

using namespace Magick;
using namespace std;



namespace openPablo
{

    /*
     * @class PSDProcessor
     *
     * @brief Abstract class to interface the capabilities of a processor
     *
     * Abstract class..
     *
     */


	PSDProcessor::PSDProcessor()
    {
        //
    }



	PSDProcessor::~PSDProcessor()
    {
        //

    }



    void PSDProcessor::start ()
    {
 	   // Initialize ImageMagick install location for Windows
 	  InitializeMagick(NULL);

 	  // TODO: do it correctly.
 	  Magick::Image magick;
 	  if (imageBlob.length() > 0)
 	  {
 	 	  qDebug() << "Opening from memory..";
 	//	  magick.read(imageBlob);
 	  }
 	  else
 	  {
 	 	  qDebug() << "Opening file: " << filename.toStdString().c_str();

 	 	  // FIXME: test for empty string


 	 	  // determine if user wants to have second (original) layer (..)

	 	  magick.read(filename.toStdString());

	 	  list<Image> layers;

	 	  // copy original image as layer
	 	  layers.push_back (magick);

	 	  // create next optimized layer
	 	  magick.renderingIntent(Magick::PerceptualIntent);
	 	  magick.unsharpmask(40, 5.0, 1.0, 0);
	      magick.normalize();
  	  	  magick.gamma(1.8);
	 	  layers.push_back (magick);

	 	  Image finalPSD;
	      std::string inputFile = filename.toStdString();
	 	  writeImages( layers.begin(), layers.end(), "/tmp/" + inputFile, true );
	      //finalPSD.write(



	 	  // if user selected a special layer, read PSD file as layered image
	 	  // and extract the correct layer

	 	  // read layers
// 	 	  list<Image> layers;
// 	 	  readImages(&layers, filename.toStdString());
// 	 	  //
//  	     std::string inputFile = filename.toStdString();
// 	 	  int curLayer = 0;
// 	 	  for (list<Image>::iterator it = layers.begin(); it != layers.end(); it++)
// 	 	  {
// 	 	 	  qDebug() << "Writing layer..";
// 	 	 	  QString layerName = QString::number(curLayer++);
// 	 		  it -> write("/tmp/" + inputFile + layerName.toStdString() + ".jpg");
// 	 	  }
 	  }

    }



    void PSDProcessor::setBLOB (unsigned char *data, uint64_t datalength)
    {
    	// create blob
		imageBlob.updateNoCopy(data, datalength );
    }
}

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

#include "Engine.hpp"
#include "EngineFactory.hpp"
#include "ImageProcessor.hpp"

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


#include <boost/property_tree/ptree.hpp>


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

            std::string processLayersStr = pt.get<std::string>("Processors.PSD.ProcessLayers");

            // TODO: some more general "YES,yes,True,TRUE,true,1" routine
            if (processLayersStr == "Yes")
            {
                std::string layersStr = pt.get<std::string>("Processors.PSD.Layers");
                std::cout << "Processing all Layers with Pattern " << layersStr  << "\n";

                //


                // if user selected a special layer, read PSD file as layered image
                // and extract the correct layer

                // read layers
                list<Image> layers;
                readImages(&layers, filename.toStdString());
                //
                std::string inputFile = filename.toStdString();
                int curLayer = 0;
                for (list<Image>::iterator it = layers.begin(); it != layers.end(); it++)
                {
                    qDebug() << "Writing layer..";
                    QString layerName = QString::number(curLayer++);
                    it -> write("/tmp/" + inputFile + layerName.toStdString() + ".jpg");
                }

                // now pipe the layer through the ImageProcessor
            }
            else
            {
                // no, handle PSD as normal container
                // so its ok to process it via normal image processor
                ImageProcessor *imageProcessor = new ImageProcessor();
                imageProcessor->setFilename(filename);

                imageProcessor-> setSettings (pt);
                imageProcessor-> start ();

                // destruct processor again
                delete imageProcessor;
            }
        }

    }



    void PSDProcessor::setBLOB (unsigned char *data, uint64_t datalength)
    {
        // create blob
        imageBlob.updateNoCopy(data, datalength );
    }
}

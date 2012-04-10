/*
 *  ProcessorFactory.cpp
 *
 *
 *  This file is part of openPalo.
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


/*
 * @mainpage ProcessorFactory
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file ProcessorFactory.hpp
 *
 * @brief description in brief.
 *
 */


#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <magic.h>


#include "ProcessorFactory.hpp"

#include "ImageProcessor.hpp"
#include "PDFProcessor.hpp"



namespace openPablo
{

    /*
     * @class ProcessorFactory
     *
     * @brief Factory class to create the right processor for a given image
     *
     * This will give back the correct processor when provided with an image
     * in form of filename.
     *
     */

    Processor* ProcessorFactory::createInstance (QString imageFileName, settings)
    {
        ImageProcessor *imageProcessor = new ImageProcessor();

        qDebug() << "Analysing file type of file " << imageFileName;


        // determine type of image
        // apply magic for it
        const char *magic_full;
        magic_t magic_cookie;

        magic_cookie = magic_open(MAGIC_MIME);

        if (magic_cookie == NULL)
        {
            qDebug() << "unable to initialize magic library";
            return imageProcessor;
        }
        printf("Loading default magic database\n");
        if (magic_load(magic_cookie, NULL) != 0)
        {
            qDebug() << "cannot load magic database" << magic_error(magic_cookie);
            magic_close(magic_cookie);
            return imageProcessor;
        }
        magic_full = magic_file(magic_cookie, imageFileName.toStdString().c_str());

        // convert to qstring
        QString mimeType(magic_full);
        qDebug() << "Filetype: " << mimeType;
        magic_close(magic_cookie);

        // create processor
        if (mimeType.contains("application/pdf", Qt::CaseInsensitive))
        {
        	qDebug() << "  Determined file type PDF.";

        	// create PDF Processor
            PDFProcessor *pdfProcessor = new PDFProcessor();
            pdfProcessor->setFilename(imageFileName);
        	return pdfProcessor;
        }

        if (mimeType.contains("image/jpeg", Qt::CaseInsensitive))
        {
        	qDebug() << "  Determined file type JPG.";

        	// create PDF Processor
            ImageProcessor *imageProcessor = new ImageProcessor();
            imageProcessor->setFilename(imageFileName);
        	return imageProcessor;
        }

        // assign correct filename to processor

        // unable to do anything
    	qDebug() << "  Unknown file type.";


        return imageProcessor;
    }

}

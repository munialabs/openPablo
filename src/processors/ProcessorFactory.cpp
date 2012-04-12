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


#include <string.h>
#include <QString>
#include <QFile>
#include <QDataStream>
#include <QDebug>

#include <magic.h>
#include "libraw/libraw.h"
#include "boost/format.hpp"


#ifdef WIN32
#define snprintf _snprintf
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <netinet/in.h>
#endif


#include "ProcessorFactory.hpp"

#include "ImageProcessor.hpp"
#include "PDFProcessor.hpp"


using boost::format;


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

    Processor* ProcessorFactory::createInstance (QString imageFileName)
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


        // create processors depending on filetype


        // --- RAW section
        //
        LibRaw iProcessor;

        // Open the file and read the metadata
        if (iProcessor.open_file(imageFileName.toStdString().c_str()) == LIBRAW_SUCCESS)
        {
        	// could be a RAW
        	qDebug() << "  Determined file type RAW.";

            // unpack and develop with dcraw
        	int errorCode;
            iProcessor.unpack();
            iProcessor.dcraw_process();
            libraw_processed_image_t *developedImage = iProcessor.dcraw_make_mem_image(&errorCode);

            // convert developed libraw image to something magic can handle-- e.g. ppm.
            std::string header;
            int maxValue = (1 << developedImage ->bits)-1;
            header = boost::str (boost::format ("P6\n%d %d\n%d\n") % developedImage ->width % developedImage -> height % maxValue);
            unsigned char *ppmBuffer = new unsigned char [developedImage -> data_size + header.size()];
            memcpy (ppmBuffer, header.data(), header.size());
			   /*
				 NOTE:
				 data in img->data is not converted to network byte order.
				 So, we should swap values on some architectures for dcraw compatibility
				 (unfortunately, xv cannot display 16-bit PPMs with network byte order data
			   */
			   #define SWAP(a,b) { a ^= b; a ^= (b ^= a); }
				   if (developedImage->bits == 16 && htons(0x55aa) != 0x55aa)
					   for(unsigned i=0; i< developedImage->data_size; i+=2)
						   SWAP(developedImage->data[i],developedImage->data[i+1]);
			   #undef SWAP


		    // copy image data
			memcpy (ppmBuffer+header.size(), developedImage->data, developedImage->data_size);

            // Finally, let us free the image processor for work with the next image
            iProcessor.recycle();

        	// create JPG Processor
            ImageProcessor *imageProcessor = new ImageProcessor();

            // FIXME: still set filename for now for ooutput reasons.
            imageProcessor->setFilename(imageFileName);
            imageProcessor -> setBLOB (ppmBuffer, developedImage->data_size+header.size());
        	return imageProcessor;
        }


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

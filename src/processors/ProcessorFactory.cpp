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

#include "ProcessorFactory.hpp"

#include "ImageProcessor.hpp"


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

	   	qDebug() << "Analysing file type of file ";

	    QFile file(imageFileName);
	    if (!file.open(QIODevice::ReadOnly) )
	    {
	      qDebug() << ("failed to load ");// << iccPath << "/" << outputICC << "\n";
	      return imageProcessor;
	    }

	    // Read and check the header
	    QDataStream in(&file);
	    quint32 magic;
	    in >> magic;
	    file.close();

	    // -----

	    // check if its a pdf file
	    if ( magic == 2064261152)
	    {
	      // this is a pdf file
//	      std::cout << "  Found PDF File.\n";
	    }



      // determine type of image
          
    // for now just PDF file
//     const char *magic_full;
//     magic_t magic_cookie;
//     /*MAGIC_MIME tells magic to return a mime of the file, but you can specify different things*/
//     magic_cookie = magic_open(MAGIC_MIME);
//     
//     if (magic_cookie == NULL) {
//             printf("unable to initialize magic library\n");
//             return 1;
//             }
//         printf("Loading default magic database\n");
//         if (magic_load(magic_cookie, NULL) != 0) {
//             printf("cannot load magic database - %s\n", magic_error(magic_cookie));
//             magic_close(magic_cookie);
//             return 1;
//         }
//     magic_full = magic_file(magic_cookie, inputFile.c_str());
//     std::cout << "Filetype: " << magic_full;
//     magic_close(magic_cookie);
//     

	  // create processor

	  // assign correct filename to processor

      return imageProcessor;
   }
    
}

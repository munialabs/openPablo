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

/***************************************************************************
 *   Copyright (C) 2005 by Dominik Seichter                                *
 *   domseichter@web.de                                                    *
 ***************************************************************************/


#include "PDFProcessor.hpp"


#include <sys/stat.h>
#include <stdlib.h>
#include <cstdio>
#include <QString>
#include <QDebug>

#include <podofo/podofo.h>



using namespace std;
using namespace PoDoFo;


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


    PDFProcessor::PDFProcessor()
    {
        //
    }



    PDFProcessor::~PDFProcessor()
    {
        //

    }



    void PDFProcessor::start ()
    {
        int nNum = 0;
        try
        {
            PdfObject*  pObj  = NULL;

            // open document
            qDebug() << "Opening file: " << filename.toStdString().c_str();
            PdfMemDocument document( filename.toStdString().c_str() );

//    	    m_pszOutputDirectory = const_cast<char*>(pszOutput);

            TCIVecObjects it = document.GetObjects().begin();

            while( it != document.GetObjects().end() )
            {
                if( (*it)->IsDictionary() )
                {
                    PdfObject* pObjType = (*it)->GetDictionary().GetKey( PdfName::KeyType );
                    PdfObject* pObjSubType = (*it)->GetDictionary().GetKey( PdfName::KeySubtype );
                    if( ( pObjType && pObjType->IsName() && ( pObjType->GetName().GetName() == "XObject" ) ) ||
                            ( pObjSubType && pObjSubType->IsName() && ( pObjSubType->GetName().GetName() == "Image" ) ) )
                    {
                        pObj = (*it)->GetDictionary().GetKey( PdfName::KeyFilter );
                        if( pObj && pObj->IsArray() && pObj->GetArray().GetSize() == 1 &&
                                pObj->GetArray()[0].IsName() && (pObj->GetArray()[0].GetName().GetName() == "DCTDecode") )
                            pObj = &pObj->GetArray()[0];

                        std::string filterName = pObj->GetName().GetName();
                        bool processed = 0;

                        if( pObj && pObj->IsName() && ( filterName == "DCTDecode" ) )
                        {
                            // The only filter is JPEG -> create a JPEG file
                            qDebug() << "JPG found.\n";
                            processed = true;
                            nNum++;
                        }

                        if( pObj && pObj->IsName() && ( filterName == "JPXDecode" ) )
                        {
                            // The only filter is JPEG -> create a JPEG file
                            qDebug() << "JPG found.\n";
                            processed = true;
                            nNum++;
                        }

                        if( pObj && pObj->IsName() && ( filterName == "FlateDecode" ) )
                        {
                            // The only filter is JPEG -> create a JPEG file
                            qDebug() << "JPG found.\n";
                            processed = true;
                            nNum++;
                        }


                        // else we found something strange, we do not care about it for now.
                        if (processed == false)
                        {
                            qDebug() << "Unknown image type found:" << QString::fromStdString(filterName) << "\n";
                            nNum++;
                        }

                        document.FreeObjectMemory( *it );
                    }
                }

                ++it;
            }


        }
        catch( PdfError & e )
        {
            qDebug() << "Error: An error ocurred during processing the pdf file:" << e.GetError();
            e.PrintErrorMsg();
            return;// e.GetError();
        }

        // TODO: statistics of no of images etc
//      nNum = extractor.GetNumImagesExtracted();

        qDebug() << "Extracted " << nNum << " images successfully from the PDF file.\n";
    }


    void PDFProcessor::setBLOB (unsigned char *data, uint64_t datalength)
    {
        //
    }
}

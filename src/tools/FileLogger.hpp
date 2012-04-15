/*
 *  FileLogger.hpp
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


#ifndef OPENPABLO_FileLOGGER_H_
#define OPENPABLO_FileLOGGER_H_

/*
 * @mainpage FileLogger
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file FileLogger.hpp
 *
 * @brief description in brief.
 *
 */


#include <QString>
#include <stdint.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/string_path.hpp>
#include <boost/property_tree/json_parser.hpp>

#include "logog/logog.hpp"


using namespace logog;


namespace openPablo
{

    /*
     * @class FileLogger
     *
     * @brief Abstract class to interface the capabilities of a engine
     *
     * Abstract class..
     *
     */
    class FileLogger: public logog::Target
    {
        public:
            /*
             *
             */
            virtual int Output	(	const LOGOG_STRING & 	data	)	;

            virtual ~FileLogger();


    };

}


#endif // OPENPABLO_FileLOGGER_H_

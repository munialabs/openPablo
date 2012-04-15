/*
 *  HTMLLogger.cpp
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

#include "HTMLLogger.hpp"


/*
 * @mainpage HTMLLogger
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file HTMLLogger.cpp
 *
 * @brief description in brief.
 *
 */



namespace openPablo
{

    /*
     * @class HTMLLogger
     *
     * @brief Abstract class to interface the capabilities of a processor
     *
     * Abstract class..
     *
     */

//	int HTMLLogger::Receive	(	const Topic & 	topic	)
//	{
//		//
//
//		return 0;
//	}
//


    int HTMLLogger::Output	(	const LOGOG_STRING & 	data	)
    {
        //
        std::cout << "IN HTML: -------------- " << data.c_str();
        return 0;
    }



    HTMLLogger::~HTMLLogger()
    {
        //
    }


}

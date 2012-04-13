/*
 *  EngineFactory.hpp
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



#ifndef OPENPABLO_ENGINEFACTORY_H_
#define OPENPABLO_ENGINEFACTORY_H_

/*
 * @mainpage EngineFactory
 *
 * Description in html
 * @author Aydin Demircioglu
 */


/*
 * @file EngineFactory.hpp
 *
 * @brief description in brief.
 *
 */


#include <QString>


#include "Engine.hpp"


using namespace openPablo;


namespace openPablo
{

    /*
     * @class EngineFactory
     *
     * @brief Factory class to create the right engine for a given image
     *
     * This will give back the correct engine when provided with an image
     * in form of filename.
     *
     */
    class EngineFactory
    {
        public:
            /*
             *
             */
            static Engine* createEngine (QString engineName);

    };

}


#endif // OPENPABLO_ENGINEFACTORY_H_

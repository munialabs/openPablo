/*
 *  MagickEngine.cpp
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

#include "MagickEngine.hpp"

#include <Magick++.h>
#include <magick/MagickCore.h>
#include <string>
#include <QString>
#include <QDebug>
#include <QDir>


/*
 * @mainpage MagickEngine
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file MagickEngine.cpp
 *
 * @brief description in brief.
 *
 */

using namespace Magick;



namespace openPablo
{

    /*
     * @class MagickEngine
     *
     * @brief Abstract class to interface the capabilities of a processor
     *
     * Abstract class..
     *
     */


    MagickEngine::MagickEngine()
    {
        //
    }



    MagickEngine::~MagickEngine()
    {
        //
    }



    void MagickEngine::start ()
    {
        // Initialize ImageMagick install location for Windowsm
        // just to be sure
        InitializeMagick(NULL);

        // create next optimized layer
        magickImage.renderingIntent(Magick::PerceptualIntent);
        magickImage.unsharpmask(40, 5.0, 1.0, 0);
        magickImage.normalize();
        magickImage.gamma(2.8);
        magickImage.equalize();

    }



    void MagickEngine::setMagickImage (Magick::Image _magickImage)
    {
        // easy as we do not need to convert
        magickImage = _magickImage;
    }



    Magick::Image MagickEngine::getMagickImage ()
    {
        // easy as we do not need to convert
        return magickImage;
    }


}

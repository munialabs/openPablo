/*
 *  PSDProcessor.hpp
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


#ifndef OPENPABLO_PSDPROCESSOR_H_
#define OPENPABLO_PSDPROCESSOR_H_

/*
 * @mainpage PSDProcessor
 *
 * Description in html
 * @author Aydin Demircioglu
  */


/*
 * @file PSDProcessor.hpp
 *
 * @brief description in brief.
 *
 */


#include <QString>

#include <Magick++.h>

#include "Processor.hpp"


using namespace Magick;


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
	class PSDProcessor: public Processor
	{
	public:
	  /*
	   *
	   */

	  PSDProcessor();

	  virtual ~PSDProcessor();

	  virtual void start ();

	  virtual void setBLOB (unsigned char *data, uint64_t datalength);


	private:
	  Blob imageBlob;
	};

}


#endif

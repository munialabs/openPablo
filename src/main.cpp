/*
 *  main.cpp
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



#include <cv.h>
#include <highgui.h>


#include "Processor.hpp"
#include "ProcessorFactory.hpp"


#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QTextStream>



//#include "rtengine.h"

#include <string>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <exception>


#include "lensfun.h"
#include <exiv2/exiv2.hpp>

#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/string_path.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>


#include "lcms2.h"
#include "lcms2_plugin.h"


using namespace std;


using namespace openPablo;





void customMessageHandler(QtMsgType type, const char *msg)
{
    QString txt;
    switch (type)
    {
        case QtDebugMsg:
            txt = QString("Debug: %1").arg(msg);
            break;

        case QtWarningMsg:
            txt = QString("Warning: %1").arg(msg);
            break;
        case QtCriticalMsg:
            txt = QString("Critical: %1").arg(msg);
            break;
        case QtFatalMsg:
            txt = QString("Fatal: %1").arg(msg);
            abort();
            break;
    }

    QFile outFile("/tmp/debuglog.txt");
    outFile.open(QIODevice::WriteOnly | QIODevice::Append);
    QTextStream ts(&outFile);
    ts << txt << endl;
}



int main ( int argc, char **argv )
{
    // some logging initialization
//	qInstallMsgHandler(customMessageHandler);


//    rtengine::Settings mySettings;
//    rtengine::init (&s, ".");


    // TODO: crashreporter-lib..

    // TODO: arguments interpretation
    // for now: just test for 1 argument with the ticket.

    // FIXME: version number
    std::cout << "\nopenPablo v0.1\n";

    if (argc != 2)
    {
        std::cout << "\n  You must only specify one ticket!\n";
        return (-1);
    }


    using boost::property_tree::ptree;
    ptree pt;

    // Load the settings file into the property tree. If reading fails
    // (cannot open file, parse error), an exception is thrown.

    // FIXME: try json first then xml or info parser.
    // TODO: info parser should be #1 way of specifiying tickets.
    read_json(argv[1] , pt);
    write_info ("/tmp/data/settings.info", pt);

    // check if input

    std::string iccPath = pt.get<std::string>("Settings.Color.ICC.Path");
    std::cout << iccPath << "\n";

    std::string outputICC = pt.get<std::string>("Settings.Color.ICC.Output");
    std::cout << outputICC << "\n";

    std::string inputFile = pt.get<std::string>("Input.InputFile");
    std::string inputPath = pt.get<std::string>("Input.InputPath");

    // FIXME: check if file exists


    // depending on file type different things should happen.
    QDir inputDir (QString::fromStdString(pt.get<std::string>("Input.InputPath")));
    QString imageFileName = inputDir.filePath(QString::fromStdString(pt.get<std::string>("Input.InputFile")));

    Processor *processor = ProcessorFactory::createInstance (imageFileName);


    std::cout << imageFileName.toStdString() << "\n";
    std::string inputFileFull = imageFileName.toStdString();


    processor -> setSettings (pt);
    //    processor.setEngine ("GIMP");
    processor -> start ();

    // destruct processor again
    delete processor;

    // inputPath + "/" + inputFile
    //QString::fromStdString(argv[1]);


    return -1;


    // ----
    QFile iccfile(QString::fromStdString(iccPath + "/" + outputICC));

    QByteArray outputProfile;
    if(iccfile.open(QIODevice::ReadOnly))
    {
        outputProfile = iccfile.readAll();
        iccfile.close();
    }
    else
    {
        std::cout << ("failed to load ") << iccPath << "/" << outputICC << "\n";
        return -1;
    }


//    PdfStreamedDocument document( "tests/tmp.pdf" );

    return -1;
    /*



      try {


      }
      catch( std::exception &error_ )
        {
          cout << "Caught exception: " << error_.what() << endl;
          return 1;
        }
    */
    return 0;
}

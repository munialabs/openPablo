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


#include "FileLogger.hpp"
#include "HTMLLogger.hpp"

#include "Processor.hpp"
#include "ProcessorFactory.hpp"


#include <QDir>
#include <QFile>
#include <QDataStream>
#include <QTextStream>
#include <QDebug>


//#include "rtengine.h"

#include <string>
#include <iostream>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <exception>


//#include "lensfun.h"

#include "logog.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/string_path.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/format.hpp>
#include <boost/filesystem.hpp>


#include "lcms2.h"
#include "lcms2_plugin.h"


using namespace std;
using namespace openPablo;
using namespace logog;




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


void testFile(QString _filename)
{
    QFile imageFile( _filename);
    if (!( imageFile.exists() && imageFile.open( QIODevice::ReadOnly)))
    {
        // it could not be opened
        qDebug() << "Either file" << _filename << "does not exist or could not be opened.\n";
        throw;
    }

    imageFile.close();
}


int main ( int argc, char **argv )
{
    LOGOG_INITIALIZE();


    int retValue = 0;
    try
    {
        Cout out;
        HTMLLogger htmlLogger;
        FileLogger fileLogger;

        // some logging initialization
        //	qInstallMsgHandler(customMessageHandler);


        //    rtengine::Settings mySettings;
        //    rtengine::init (&s, ".");


        // TODO: crashreporter-lib..

        // TODO: arguments interpretation
        // for now: just test for 1 argument with the ticket.

        // FIXME: version number

        INFO("openPablo v0.1");

        if (argc != 2)
        {
            ERR("You must only specify one ticket!");
            return (-1);
        }


        using boost::property_tree::ptree;
        ptree pt;

        // Load the settings file into the property tree. If reading fails
        // (cannot open file, parse error), an exception is thrown.

        // FIXME: try json first then xml or info parser.
        // TODO: info parser should be #1 way of specifiying tickets.
        read_json(argv[1] , pt);
//	    write_info ("/tmp/data/settings.info", pt);


        // --- read the input file

        QDir inputDir (QString::fromStdString(pt.get<std::string>("Input.InputPath")));
        QString imageFullName = inputDir.filePath(QString::fromStdString(pt.get<std::string>("Input.InputFile")));

        // FIXME: check if file exists
        testFile(imageFullName);

        // depending on file type different things should happen. job of the processor.
        Processor *processor = ProcessorFactory::createInstance (imageFullName);

        processor -> setSettings (pt);
        processor -> start ();

        // destruct processor again
        delete processor;
    }
    catch( std::exception &error_ )
    {
        cout << "Caught exception: " << error_.what() << endl;
        retValue = 1;
    }

    LOGOG_SHUTDOWN();

    return retValue;
}

/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>. */

#include "Darkroom.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KDE/KLocale>
#include <KUrl>

static const char description[] =
    I18N_NOOP("A raw decoder.");

static const char version[] = "1.3";

int main(int argc, char **argv)
{
    KAboutData about("darkroom", 0, ki18n("Darkroom"), version, ki18n(description),
                     KAboutData::License_LGPL, ki18n("(c) 2008 Cyrille Berger"), KLocalizedString(), 0, "cberger@cberger.net");
    about.addAuthor( ki18n("Cyrille Berger"), KLocalizedString(), "cberger@cberger.net" );
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Document to open" ));
    KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

    Darkroom *widget = new Darkroom;

    // see if we are starting with session management
    if (app.isSessionRestored())
    {
        RESTORE(Darkroom);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if (args->count() == 0)
        {
            //darkroom *widget = new darkroom;
            widget->show();
        }
        else
        {
            widget->show();
            widget->openUrl( args->url( 0 ).toLocalFile(  ) );
//             int i = 0;
//             for (; i < args->count(); i++)
//             {
                //darkroom *widget = new darkroom;
//                 widget->show();
//             }
        }
        args->clear();
    }

    return app.exec();
}

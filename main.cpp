/*
    This file is part of Akonadi.

    Copyright (c) 2006 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/

#include <kaboutdata.h>
#include <qapplication.h>

#include "instanceselector.h"

#include <Kdelibs4ConfigMigrator>
#include <KDBusService>
#include <stdlib.h>
#include <QCommandLineParser>
#include <QCommandLineOption>

int main(int argc, char **argv)
{
    Kdelibs4ConfigMigrator migrate(QLatin1String("akonadiconsole"));
    migrate.setConfigFiles(QStringList() << QLatin1String("akonadiconsolerc"));
    migrate.setUiFiles(QStringList() << QLatin1String("akonadiconsoleui.rc"));
    migrate.migrate();

    QApplication app(argc, argv);
    KAboutData aboutData(QStringLiteral("akonadiconsole"),
                         i18n("Akonadi Console"),
                         QStringLiteral("0.99"),
                         i18n("The Management and Debugging Console for Akonadi"),
                         KAboutLicense::GPL,
                         i18n("(c) 2006-2014 the Akonadi developer"),
                         QStringLiteral("http://pim.kde.org/akonadi/"));
    QApplication::setWindowIcon(QIcon::fromTheme(QLatin1String("akonadi")));
    aboutData.addAuthor(i18n("Tobias König"), i18n("Author"), QStringLiteral("tokoe@kde.org"));
    aboutData.addAuthor(i18n("Volker Krause"),  i18n("Author"), QStringLiteral("vkrause@kde.org"));
    KAboutData::setApplicationData(aboutData);

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);
    parser.addOption(QCommandLineOption(QStringList() << QLatin1String("remote"), i18n("Connect to an Akonadi remote debugging server"), "server"));

    KDBusService service;

    if (parser.isSet("remote")) {
        const QString akonadiAddr = QStringLiteral("tcp:host=%1,port=31415").arg(parser.value("remote"));
        const QString dbusAddr = QStringLiteral("tcp:host=%1,port=31416").arg(parser.value("remote"));
        setenv("AKONADI_SERVER_ADDRESS", akonadiAddr.toLatin1(), 1);
        setenv("DBUS_SESSION_BUS_ADDRESS", dbusAddr.toLatin1(), 1);
    }

    InstanceSelector instanceSelector(parser.isSet("remote") ? parser.value("remote") : QString());

    return app.exec();
}

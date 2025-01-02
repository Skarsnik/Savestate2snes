/*
    This file is part of the SaveState2snes software
    Copyright (C) 2017  Sylvain "Skarsnik" Colinet <scolinet@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "savestate2snesw.h"
#include "trainingtimer.h"
#include <QApplication>
#include <QMessageBox>
#include "firsttimedialog.h"
#include "shortcuteditdialog.h"

static QTextStream logfile;
static QTextStream lowlogfile;
static QTextStream cout(stdout);
bool    dontLogNext = false;

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    QTextStream*    log = &logfile;
    //cout << msg;
    if (dontLogNext)
        return ;
    QString logString = QString("%6 %5 - %7: %1 \t(%2:%3, %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function).arg(context.category, 20).arg(QDateTime::currentDateTime().toString(Qt::ISODate));
    if (QString(context.category).left(8) == "LowLevel")
    {
        //cout << "log is lowlevel. Writing to lowlevelfile";
        log = &lowlogfile;
    }
    switch (type)
    {
        case QtDebugMsg:
            *log << logString.arg("Debug");
            break;
        case QtCriticalMsg:
            *log << logString.arg("Critical");
            break;
        case QtWarningMsg:
            *log << logString.arg("Warning");
            break;
        case QtFatalMsg:
            *log << logString.arg("Fatal");
            *log<< "\n"; log->flush();
            QMessageBox::critical(NULL, QObject::tr("Critical error"), msg);
            qApp->exit(1);
            break;
        case QtInfoMsg:
            *log << logString.arg("Info");
            break;
    }
    *log << "\n";
    log->flush();
#ifdef QT_DEBUG
    if (QString(context.category) == "Telnet")
    {
        //cout << "Writing to lowlevelfile";
        lowlogfile << logString.arg("MSG");
        lowlogfile << "\n";
        lowlogfile.flush();
    }
#endif
    if (log != &lowlogfile)
    {
        //cout << logString;
        cout << QString("%1 : %2").arg(context.category, 20).arg(msg) << "\n";
        cout.flush();
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QDir("/").mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QFile   mlog(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/log.txt");
    QFile   mlowlog(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/loglow.txt");
    logfile.setDevice(&mlog);
    lowlogfile.setDevice(&mlowlog);
    if (mlog.open(QIODevice::WriteOnly | QIODevice::Text))
    {
#ifdef QT_DEBUG
        mlowlog.open(QIODevice::WriteOnly | QIODevice::Text);
#endif
        qInstallMessageHandler(myMessageOutput);
    }
    QApplication::setApplicationName("Savestate2SNES");
    QSettings settings("skarsnik.nyo.fr", "SaveState2SNES");
    QTranslator translator;
    QString locale = QLocale::system().name().split('_').first();
    translator.load(a.applicationDirPath() + "/i18n/savestate2snes_" + locale + ".qm");
// This is only defined in the PRO file
#ifdef GIT_TAG_VERSION
    QString plop(GIT_TAG_VERSION);
    plop.remove(0, 1); // Remove the v
    QApplication::setApplicationVersion(QVersionNumber::fromString(plop).toString());
#else
    QApplication::setApplicationVersion("0.99");
#endif
    QLoggingCategory::setFilterRules("EmuNWAccessClient.debug=true\n");
    a.installTranslator(&translator);
    if (!settings.contains("lastSaveStateDir"))
    {
        FirstTimeDialog diag;
        if (diag.exec() == QDialog::Rejected)
            return 1;
        settings.setValue("lastSaveStateDir", diag.savePath);
        settings.setValue("mode", diag.selectedMode());
    }
    Savestate2snesw w;
    w.show();
    /*TrainingTimer pt;
    pt.show();*/

    return a.exec();
}

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
#include <QApplication>
#include <QMessageBox>
#include "firsttimedialog.h"
#include "shortcuteditdialog.h"

static QTextStream logfile;
static QTextStream cout(stdout);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    //cout << msg;
    switch (type)
    {
        case QtDebugMsg:
            logfile << QString("Debug: %1 \t(%2:%3, %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
            break;
        case QtCriticalMsg:
            logfile << QString("Critical: %1 \t(%2:%3, %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
            QMessageBox::critical(NULL, QObject::tr("Critical error"), msg);
            qApp->exit(1);
            break;
        case QtWarningMsg:
            logfile << QString("Warning: %1 \t(%2:%3, %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
            break;
        case QtFatalMsg:
            logfile << QString("Fatal: %1 \t(%2:%3, %4)").arg(localMsg.constData()).arg(context.file).arg(context.line).arg(context.function);
            break;
    }
    logfile << "\n";
    logfile.flush();
    cout << msg << "\n";
    cout.flush();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    QDir("/").mkpath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    QFile   mlog(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/log.txt");
    logfile.setDevice(&mlog);
    if (mlog.open(QIODevice::WriteOnly | QIODevice::Text))
        qInstallMessageHandler(myMessageOutput);
    QSettings settings("skarsnik.nyo.fr", "SaveState2SNES");
    /*ShortcutEditDialog sdiag;
    sdiag.exec();
    return 1;*/
    if (!settings.contains("lastSaveStateDir"))
    {
        FirstTimeDialog diag;
        if (diag.exec() == QDialog::Rejected)
            return 1;
        settings.setValue("lastSaveStateDir", diag.savePath);
    }
    Savestate2snesw w;
    w.show();

    return a.exec();
}

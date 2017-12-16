#include "savestate2snesw.h"
#include <QApplication>
#include <QMessageBox>

static QTextStream logfile;
static QTextStream cout(stdout);

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    cout << msg;
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
    Savestate2snesw w;
    w.show();

    return a.exec();
}

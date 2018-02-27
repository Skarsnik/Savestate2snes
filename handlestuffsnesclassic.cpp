#include <QLoggingCategory>
#include <QThread>
#include "handlestuffsnesclassic.h"

#define CLOVERSAVESTATEPATH "/tmp/savestate2snes.svt"
#define CLOVERROLLBACKPATH "/tmp/rollback/"
#define SCREENSHOTPATH "/tmp/savestate2snes.png"
#define KILLCANOECMD "kill -2 `pidof canoe-shvc`"

Q_LOGGING_CATEGORY(log_handleSNESClassic, "HSSnesClassic")

#define sDebug() qCDebug(log_handleSNESClassic)

// cp $statefile $rollback$/
// tar czf /tmp/rollback.tar.gz $rollbackpath



HandleStuffSnesClassic::HandleStuffSnesClassic()
{
    ftpCo = new MiniFtp();
}

QStringList HandleStuffSnesClassic::getCanoeExecution()
{
    QByteArray ba = telCo->syncExecuteCommand("ps | grep canoe | grep -v grep");
    QString result = ba.trimmed();
    QString canoeStr = result.mid(result.indexOf("canoe-shvc"));
    sDebug() << "Canoe Str : " << canoeStr;
    QStringList canoeArgs = canoeStr.split(" ");
    return canoeArgs;
}

void    HandleStuffSnesClassic::killCanoe(int signal = 2)
{
    QString canoePid = telCo->syncExecuteCommand("pidof canoe-shvc").trimmed();
    telCo->syncExecuteCommand(QString("kill -%1 `pidof canoe-shvc`").arg(signal));
    telCo->syncExecuteCommand("test -e /tmp/plop || (sleep 1 && kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop)");
    telCo->syncExecuteCommand("wait " + canoePid);
}

void    HandleStuffSnesClassic::runCanoe(QStringList canoeArgs)
{
    sDebug() << "Running canoe : " << canoeArgs;
    canoeCo->executeCommand(canoeArgs.join(" ") + " 2>/dev/null");
}

void    HandleStuffSnesClassic::removeCanoeUnnecessaryArg(QStringList &canoeRun)
{
    QStringList optArgToremove;
    optArgToremove << "-rollback-snapshot-period" << "--load-time-path" << "-rollback-input-dir"  << "-rollback-output-dir" << "--save-time-path" << "--wait-transition-fd";
    optArgToremove << "--finish-transition-fd" << "--start-transition-fd" << "--rollback-ui" << "-rollback-snapshot-period" << "-rollback-mode";
    foreach( QString arg, optArgToremove)
    {
        int i = canoeRun.indexOf(arg);
        if (i != -1)
        {
            qDebug() << "Removing : " << arg;
            canoeRun.removeAt(i);
            canoeRun.removeAt(i);
        }
    }
    int i = canoeRun.indexOf("--enable-sram-file-hash");
    if (i != -1)
        canoeRun.removeAt(i); 
}

QByteArray HandleStuffSnesClassic::saveState(bool trigger)
{
    Q_UNUSED(trigger)
    sDebug() << "Savestate";
    QStringList canoeRun = getCanoeExecution();
    if (canoeRun.at(0) != "canoe-shvc")
        return QByteArray();
    /*QString fileSavePath = canoeRun.at(canoeRun.indexOf("--save-time-path") + 1);
    QString rollbackDir = canoeRun.at(canoeRun.indexOf("-rollback-output-dir") + 1);*/
    int soq = canoeRun.indexOf("--save-on-quit");
    if (soq == -1)
    {
        sDebug() << "First run inside savestate2snes";
        killCanoe();
        //  First we change what rollback file to load
        int spos = canoeRun.indexOf("--save-time-path");
        int ripos = canoeRun.indexOf("-rollback-input-dir");
        int ropos = canoeRun.indexOf("-rollback-output-dir");
        int lpos = canoeRun.indexOf("--load-time-path");
        if (lpos == -1)
        {
            canoeRun.append("--load-time-path");
            canoeRun.append(canoeRun.at(spos + 1));
        } else {
            canoeRun[lpos + 1] = canoeRun.at(spos + 1);
        }
        if (ripos == - 1)
        {
            canoeRun.append("--rollback-input-dir");
            canoeRun.append(canoeRun.at(ropos + 1));
        } else {
            canoeRun[ripos + 1] = canoeRun.at(ropos + 1);
        }

        // probably useless
        canoeRun.removeAt(canoeRun.indexOf("--enable-sram-file-hash"));

        // Adding save savestate
        canoeRun.append("--save-on-quit");
        canoeRun.append(CLOVERSAVESTATEPATH);
        int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
        canoeRun.removeAt(sshot);
        canoeRun.removeAt(sshot);
        canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
        runCanoe(canoeRun);
        //return QByteArray();
        QThread::msleep(400);
    }
    sDebug() << "Killing canoe to restart";
    killCanoe();
    /*telCo->syncExecuteCommand(QString("cp %1 %2 && cd %2/../ && tar czf /tmp/rollback.tar.gz rollback/ && cd ~/").arg(fileSavePath).arg(rollbackDir));
    if (rollbackDir != "/tmp/rollback/")
        telCo->syncExecuteCommand("cp -r " + rollbackDir + "/* /tmp/rollback/");*/
    QThread::msleep(200);
    telCo->syncExecuteCommand("ls -l " + QString(CLOVERSAVESTATEPATH));
    QByteArray toret = ftpCo->get(CLOVERSAVESTATEPATH);
    if (canoeRun.indexOf("-resume") == -1)
    {
        canoeRun.append("-resume");
        canoeRun.append(CLOVERSAVESTATEPATH);
    }
    if (soq == -1)
    {
        removeCanoeUnnecessaryArg(canoeRun);
        canoeRun.append("-resume");
        canoeRun.append(CLOVERSAVESTATEPATH);
    }
    sDebug() << "Restarting Canoe";
    runCanoe(canoeRun);
    return toret;
}

void HandleStuffSnesClassic::loadState(QByteArray data)
{
    QStringList  canoeRun = getCanoeExecution();
    if (canoeRun.at(0) != "canoe-shvc")
        return;
    sDebug() << "Loading State canoerun : " << canoeRun;
    int soq = canoeRun.indexOf("--save-on-quit");
    if (soq == -1)
    {
        killCanoe();
        removeCanoeUnnecessaryArg(canoeRun);
        canoeRun.append("--save-on-quit");
        canoeRun.append(CLOVERSAVESTATEPATH);
        int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
        canoeRun.removeAt(sshot);
        canoeRun.removeAt(sshot);
        canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
        canoeRun.append("-resume");
        canoeRun.append(CLOVERSAVESTATEPATH);
    } else
        killCanoe(9); // We don't want to create a savestate
    // Don't write a new file when it's the same
    if (canoeRun.indexOf("-resume") == -1)
    {
        canoeRun.append("-resume");
        canoeRun.append(CLOVERSAVESTATEPATH);
    }
    if (lastLoadMD5 != QCryptographicHash::hash(data, QCryptographicHash::Md5))
        ftpCo->put(CLOVERSAVESTATEPATH, data);
    lastLoadMD5 = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    runCanoe(canoeRun);
}

bool HandleStuffSnesClassic::hasScreenshots()
{
    return true;
}

bool HandleStuffSnesClassic::hasShortcutsEdit()
{
    return false;
}

void HandleStuffSnesClassic::setCommandCo(TelnetConnection *co, TelnetConnection *canoe)
{
    telCo = co;
    canoeCo = canoe;
    /*canoeCo = new TelnetConnection("localhost", 1023, "root", "clover");
    canoeCo->debugName = "Canoe";
    canoeCo->conneect();*/
}

void HandleStuffSnesClassic::setShortcutLoad(quint16 shortcut)
{

}

void HandleStuffSnesClassic::setShortcutSave(quint16 shortcut)
{

}

QByteArray HandleStuffSnesClassic::getScreenshotData()
{
    return ftpCo->get(SCREENSHOTPATH);
}

quint16 HandleStuffSnesClassic::shortcutLoad()
{
    return 0;
}

quint16 HandleStuffSnesClassic::shortcutSave()
{
    return 0;
}

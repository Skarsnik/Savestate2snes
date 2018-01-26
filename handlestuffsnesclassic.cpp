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
    QString canoeStr = result.mid(result.indexOf("canoe"));
    QStringList canoeArgs = canoeStr.split(" ");
    return canoeArgs;
}

void    HandleStuffSnesClassic::killCanoe()
{
    QString canoePid = telCo->syncExecuteCommand("pidof canoe-shvc").trimmed();
    telCo->syncExecuteCommand(KILLCANOECMD);
    telCo->syncExecuteCommand("test -e /tmp/plop || (sleep 1 && kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop)");
    telCo->syncExecuteCommand("wait " + canoePid);
}

void    HandleStuffSnesClassic::runCanoe(QStringList canoeArgs)
{
    sDebug() << "Running canoe : " << canoeArgs;
    canoeCo->executeCommand(canoeArgs.join(" "));
}

void    HandleStuffSnesClassic::removeCanoeUnnecessaryArg(QStringList &canoeRun)
{
    QStringList optArgToremove;
    optArgToremove << "-rollback-snapshot-period" << "--load-time-path" << "-rollback-input-dir"  << "-rollback-output-dir" << "--save-time-path" << "--wait-transition-fd";
    optArgToremove << "--finish-transition-fd" << "--start-transition-fd" << "--rollback-ui" << "-rollback-snapshot-period";
    foreach( QString arg, optArgToremove)
    {
        int i = canoeRun.indexOf(arg);
        if (i != 1)
        {
            canoeRun.removeAt(i);
            canoeRun.removeAt(i);
        }
    }
}

QByteArray HandleStuffSnesClassic::saveState(bool trigger)
{
    Q_UNUSED(trigger)
    sDebug() << "Savestate";
    QStringList canoeRun = getCanoeExecution();
    /*QString fileSavePath = canoeRun.at(canoeRun.indexOf("--save-time-path") + 1);
    QString rollbackDir = canoeRun.at(canoeRun.indexOf("-rollback-output-dir") + 1);*/
    int soq = canoeRun.indexOf("--save-on-quit");
    if (soq == -1)
    {
        sDebug() << "First run inside savestate2snes";
        killCanoe();
        canoeRun.append("--save-on-quit");
        canoeRun.append(CLOVERSAVESTATEPATH);
        int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
        canoeRun.removeAt(sshot);
        canoeRun.removeAt(sshot);
        canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
        runCanoe(canoeRun);
        //return QByteArray();
        QThread::msleep(200);
    }
    sDebug() << "Killing canoe to restart";
    killCanoe();
    /*telCo->syncExecuteCommand(QString("cp %1 %2 && cd %2/../ && tar czf /tmp/rollback.tar.gz rollback/ && cd ~/").arg(fileSavePath).arg(rollbackDir));
    if (rollbackDir != "/tmp/rollback/")
        telCo->syncExecuteCommand("cp -r " + rollbackDir + "/* /tmp/rollback/");*/
    telCo->syncExecuteCommand("ls -l " + CLOVERSAVESTATEPATH);
    QByteArray toret = ftpCo->get(CLOVERSAVESTATEPATH);
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
    killCanoe();
    int soq = canoeRun.indexOf("--save-on-quit");
    if (soq == -1)
    {
        removeCanoeUnnecessaryArg(canoeRun);
        canoeRun.append("--save-on-quit");
        canoeRun.append(CLOVERSAVESTATEPATH);
        int sshot = canoeRun.indexOf("--save-screenshot-on-quit");
        canoeRun.removeAt(sshot);
        canoeRun.removeAt(sshot);
        canoeRun << "--save-screenshot-on-quit" << SCREENSHOTPATH;
        canoeRun.append("-resume");
        canoeRun.append(CLOVERSAVESTATEPATH);
    }
    ftpCo->put(CLOVERSAVESTATEPATH, data);
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

void HandleStuffSnesClassic::setCommandCo(TelnetConnection *co)
{
    telCo = co;
    canoeCo = new TelnetConnection("localhost", 1023, "root", "clover");
    canoeCo->debugName = "Canoe";
    canoeCo->conneect();
}

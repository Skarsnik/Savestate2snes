#include <QCryptographicHash>
#include <QLoggingCategory>
#include <QThread>
#include <QTimer>
#include "handlestuffsnesclassic.h"

#define CLOVERSAVESTATEPATH "/tmp/savestate2snes.svt"
#define CLOVERROLLBACKPATH "/tmp/rollback/"
#define SCREENSHOTPATH "/tmp/savestate2snes.png"
#define KILLCANOECMD "kill -2 `pidof canoe-shvc`"

Q_LOGGING_CATEGORY(log_handleSNESClassic, "HSSnesClassic")

#define sDebug() qCDebug(log_handleSNESClassic)


HandleStuffSnesClassic::HandleStuffSnesClassic() : HandleStuff ()
{
    loadShortcut = 0;
    saveShortcut = 0;
    expectingSaveFile = false;
    commandSpy = nullptr;
    controlCo = nullptr;
    controllerStateTrigger = false;
}

void HandleStuffSnesClassic::setControlCo(StuffClient *client)
{
    sDebug() << "Set Control co called";
    controlCo = client;
    commandSpy = new QSignalSpy(controlCo, &StuffClient::commandFinished);
    connect(controlCo, &StuffClient::receivedFileSize, this, [=](unsigned int size)
    {
        sDebug() << "Received the size" << size;
        fileReceivedSize = size;
    });
    connect(controlCo, &StuffClient::newFileData, this, [=](QByteArray data) {
        if (expectingSaveFile)
        {
            saveStateData.append(data);
            //sDebug() << "Savedata size : " << saveStateData.size();
            if (saveStateData.size() == fileReceivedSize)
            {
                sDebug() << "Got the savestate file";
                emit saveStateFinished(true);
            }
        } else {
            sDebug() << "Recevied screenshot data";
            screenshotData.append(data);
            sDebug() << "Current size : " << screenshotData.size();
            if (screenshotData.size() == fileReceivedSize)
            {
                sDebug() << "Screenshot data get";
                emit screenshotDone();
            }
        }
    });
}

bool        HandleStuffSnesClassic::fakeWaitForCommand(QByteArray cmd, unsigned int timeout)
{
    controlCo->executeCommand(cmd);
    return commandSpy->wait(timeout);
}


QStringList HandleStuffSnesClassic::getCanoeExecution()
{
    if (!fakeWaitForCommand("ps | grep canoe | grep -v grep"))
        return QStringList();
    QByteArray ba = controlCo->commandDatas();
    sDebug() << ba;
    ba.replace(static_cast<char>(0), "");
    QString result = ba.trimmed();
    QString canoeStr = result.mid(result.indexOf("canoe-shvc"));
    sDebug() << "Canoe Str : " << canoeStr;
    QStringList canoeArgs = canoeStr.split(" ");
    return canoeArgs;
}

void    HandleStuffSnesClassic::killCanoe(int signal = 2)
{
    fakeWaitForCommand("pidof canoe-shvc");
    QString canoePid = controlCo->commandDatas().trimmed();
    fakeWaitForCommand(QString("kill -%1 `pidof canoe-shvc`").arg(signal).toLatin1());
    fakeWaitForCommand("test -e /tmp/plop || (sleep 1 && kill -2 `pidof ReedPlayer-Clover` && touch /tmp/plop)");
    fakeWaitForCommand("wait " + canoePid.toLatin1());
}

void    HandleStuffSnesClassic::runCanoe(QStringList canoeArgs)
{
    sDebug() << "Running canoe : " << canoeArgs;
    controlCo->detachedCommand(canoeArgs.join(" ").toLatin1() + " 2>/dev/null");
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

bool HandleStuffSnesClassic::mySaveState(bool trigger, bool noGet)
{
    sDebug() << "Savestate : trigger " << trigger << " No get : " << noGet;
    QByteArray toret;
    if (trigger == false && noGet == false)
    {
        saveStateData.clear();
        controlCo->getFile(CLOVERSAVESTATEPATH);
        return true;
    }
    QStringList canoeRun = getCanoeExecution();
    if (canoeRun.at(0) != "canoe-shvc")
        return false;
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
    /*controlCo->waitForCommand(QString("cp %1 %2 && cd %2/../ && tar czf /tmp/rollback.tar.gz rollback/ && cd ~/").arg(fileSavePath).arg(rollbackDir));
    if (rollbackDir != "/tmp/rollback/")
        controlCo->waitForCommand("cp -r " + rollbackDir + "/* /tmp/rollback/");*/
    QThread::msleep(200);
    fakeWaitForCommand("ls -l " + QByteArray(CLOVERSAVESTATEPATH));
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
    if (noGet == false)
    {
        expectingSaveFile = true;
        saveStateData.clear();
        controlCo->getFile(CLOVERSAVESTATEPATH);
    } else {
        emit saveStateFinished(true);
    }
    return true;
}


bool HandleStuffSnesClassic::saveState(bool trigger)
{
    return mySaveState(trigger, false);

}

void HandleStuffSnesClassic::loadState(QByteArray data)
{
    myLoadState(data, false);
}

bool HandleStuffSnesClassic::needByteData()
{
    return true;
}

void HandleStuffSnesClassic::myLoadState(QByteArray data, bool noPut)
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
    if (noPut == false && lastLoadMD5 != QCryptographicHash::hash(data, QCryptographicHash::Md5))
        controlCo->sendFile(CLOVERSAVESTATEPATH, data);
    lastLoadMD5 = QCryptographicHash::hash(data, QCryptographicHash::Md5);
    runCanoe(canoeRun);
    QTimer::singleShot(10, this, [=](){
        if (controllerStateTrigger)
        {
            controllerStateTrigger = false;
            emit controllerLoadStateFinished(true);
        } else {
            emit loadSaveStateFinished(true);
        }
    });
}

bool HandleStuffSnesClassic::hasScreenshots()
{
    return true; // FIXME
}

bool HandleStuffSnesClassic::hasPostSaveScreenshot()
{
    return true;
}

bool HandleStuffSnesClassic::doScreenshot()
{
    sDebug() << "Requesting screenshot";
    expectingSaveFile = false;
    screenshotData.clear();
    controlCo->getFile(SCREENSHOTPATH);
    return true;
}

bool HandleStuffSnesClassic::hasShortcutsEdit()
{
    return true;
}

void HandleStuffSnesClassic::controllerSaveState()
{
    controllerStateTrigger = true;
    mySaveState(false, true);
}

void HandleStuffSnesClassic::controllerLoadState()
{
    controllerStateTrigger = true;
    myLoadState(QByteArray(), true);
}

void HandleStuffSnesClassic::setShortcutLoad(quint16 shortcut)
{
    loadShortcut = shortcut;
}

void HandleStuffSnesClassic::setShortcutSave(quint16 shortcut)
{
    saveShortcut = shortcut;
}

QByteArray HandleStuffSnesClassic::getScreenshotData()
{
    return screenshotData;
}

quint16 HandleStuffSnesClassic::shortcutLoad()
{
    return loadShortcut;
}

quint16 HandleStuffSnesClassic::shortcutSave()
{
    return saveShortcut;
}


bool HandleStuffSnesClassic::saveState(QString path)
{
    Q_UNUSED(path);
    return false;
}

bool HandleStuffSnesClassic::loadState(QString path)
{
    Q_UNUSED(path);
    return false;
}


bool HandleStuffSnesClassic::hasMemoryWatch()
{
    return false;
}

void HandleStuffSnesClassic::startMemoryWatch()
{
    return;
}

void HandleStuffSnesClassic::stopMemoryWatch()
{
    return;
}

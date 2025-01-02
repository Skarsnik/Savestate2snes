#include <QtEndian>
#include <QLoggingCategory>
#include <QTemporaryFile>
Q_LOGGING_CATEGORY(log_handleStuffNWA, "HandleStuffNWA")

#define sDebug() qCDebug(log_handleStuffNWA)


#include "handlestuffnwaccess.h"


HandleStuffNWAccess::HandleStuffNWAccess() : HandleStuff ()
{
    load = false;
    doingState = false;
    memoryAccess = false;
    controllerStateRequest = false;
    memoryTimer.setInterval(20);
    connect(&memoryTimer, &QTimer::timeout, this, [=]{
        emuclient->cmdCoreReadMemory(gameInfos().memoryPreset.domain, gameInfos().memoryPreset.address , gameInfos().memoryPreset.size);
    });
}

void HandleStuffNWAccess::setNWAClient(EmuNWAccessClient *client)
{
    emuclient = client;
    connect(emuclient, &EmuNWAccessClient::readyRead, this, &HandleStuffNWAccess::onReplyRead);
    emuclient->cmdEmulatorInfo();
}


QByteArray HandleStuffNWAccess::getScreenshotData()
{
    return QByteArray();
}

bool HandleStuffNWAccess::hasShortcutsEdit()
{
    return true;
}

bool HandleStuffNWAccess::hasScreenshots()
{
    return false;
}

void HandleStuffNWAccess::setShortcutSave(quint16 shortcut)
{
    saveShortcut = shortcut;
}

void HandleStuffNWAccess::setShortcutLoad(quint16 shortcut)
{
    loadShortcut = shortcut;
}

quint16 HandleStuffNWAccess::shortcutSave()
{
    return saveShortcut;
}

quint16 HandleStuffNWAccess::shortcutLoad()
{
    return loadShortcut;
}

bool HandleStuffNWAccess::hasMemoryWatch()
{
    return memoryAccess;
}

void HandleStuffNWAccess::startMemoryWatch()
{
    memoryTimer.start();
}

void HandleStuffNWAccess::stopMemoryWatch()
{
    memoryTimer.stop();
}

void HandleStuffNWAccess::controllerLoadState()
{
    if (lastSaveData.isEmpty())
        return ;
    controllerStateRequest = true;
    loadState(lastSaveData);
}

void HandleStuffNWAccess::controllerSaveState()
{
    controllerStateRequest = true;
    saveState(true);
}

bool HandleStuffNWAccess::saveState(bool trigger)
{
    load = false;
    doingState = true;
    if (trigger == false)
        return true;
    emuclient->cmd("SAVE_STATE_TO_NETWORK");
    return true;
}

void HandleStuffNWAccess::loadState(QByteArray data)
{
    load = true;
    doingState = true;
    lastSaveData = data;
    emuclient->bcmd("LOAD_STATE_FROM_NETWORK", QString(), data);
}


bool HandleStuffNWAccess::saveState(QString path)
{
    Q_UNUSED(path)
    return false;
}

bool HandleStuffNWAccess::loadState(QString path)
{
    Q_UNUSED(path)
    return false;
}

bool HandleStuffNWAccess::needByteData()
{
    return true;
}

void HandleStuffNWAccess::onReplyRead()
{
    auto reply = emuclient->readReply();
    sDebug() << "Replied for " << reply.cmd;
    if (reply.cmd == "EMULATOR_INFO")
    {
        if (reply["commands"].indexOf("CORE_READ"))
            memoryAccess = true;
        return ;
    }
    if (reply.cmd == "CORE_READ")
    {
        quint64 value = 0;
        sDebug() << reply.binary;
        for (quint8 i = 0; i < reply.binary.size(); i++)
        {
            value += reply.binary.at(i) << (i * 8);
        }
        emit gotMemoryValue(value);
        return;
    }
    if (doingState == false)
        return;
    if (reply.cmd == "SAVE_STATE_TO_NETWORK")
    {
        saveStateData = reply.binary;
    }
    if (!controllerStateRequest)
    {
        if (load)
            emit loadStateFinished(!reply.isError);
        else
            emit saveStateFinished(!reply.isError);
    } else {
        if (load)
            emit controllerLoadStateFinished(!reply.isError);
        else
            emit controllerSaveStateFinished(!reply.isError);
        controllerStateRequest = false;
    }
}


bool HandleStuffNWAccess::hasPostSaveScreenshot()
{
    return false;
}

bool HandleStuffNWAccess::doScreenshot()
{
    return false;
}

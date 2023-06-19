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
    QTemporaryFile f("SaveStateSnesNWA");
    f.open();
    tempFilePath = f.fileName();
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
    controllerStateRequest = true;
    loadState(lastLoadedSave);
}

void HandleStuffNWAccess::controllerSaveState()
{
    controllerStateRequest = true;
    saveState(tempFilePath);
}

bool HandleStuffNWAccess::saveState(bool trigger)
{
    Q_UNUSED(trigger);
    return false;
}

void HandleStuffNWAccess::loadState(QByteArray data)
{
    Q_UNUSED(data);
}


bool HandleStuffNWAccess::saveState(QString path)
{
    load = false;
    doingState = true;
    lastLoadedSave = path;
    emuclient->cmdSaveState(path);
    return true;
}

bool HandleStuffNWAccess::loadState(QString path)
{
    load = true;
    doingState = true;
    lastLoadedSave = path;
    emuclient->cmdLoadState(path);
    return true;
}

bool HandleStuffNWAccess::needByteData()
{
    return false;
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
    if (!controllerStateRequest)
    {
        if (load)
            emit loadStateFinished(!reply.isError);
        else
            emit saveStateFinished(!reply.isError);
    } else {
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

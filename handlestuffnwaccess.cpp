#include <QtEndian>
#include <QLoggingCategory>
Q_LOGGING_CATEGORY(log_handleStuffNWA, "HandleStuffNWA")

#define sDebug() qCDebug(log_handleStuffNWA)


#include "handlestuffnwaccess.h"


HandleStuffNWAccess::HandleStuffNWAccess() : HandleStuff ()
{
    load = false;
    doingState = false;
    memoryAccess = false;
    memoryTimer.setInterval(20);
    connect(&memoryTimer, &QTimer::timeout, this, [=]{
        emuclient->cmdCoreReadMemory("WRAM", memoryToWatch, memorySize);
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
    return false;
}

bool HandleStuffNWAccess::hasScreenshots()
{
    return false;
}

void HandleStuffNWAccess::setShortcutSave(quint16 shortcut)
{
    Q_UNUSED(shortcut)
}

void HandleStuffNWAccess::setShortcutLoad(quint16 shortcut)
{
    Q_UNUSED(shortcut)
}

quint16 HandleStuffNWAccess::shortcutSave()
{
    return 1;
}

quint16 HandleStuffNWAccess::shortcutLoad()
{
    return 1;
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
    emuclient->cmdSaveState(path);
    return true;
}

bool HandleStuffNWAccess::loadState(QString path)
{
    load = true;
    doingState = true;
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
    if (load)
        emit loadStateFinished(!reply.isError);
    else
        emit saveStateFinished(!reply.isError);
}


bool HandleStuffNWAccess::hasPostSaveScreenshot()
{
    return false;
}

bool HandleStuffNWAccess::doScreenshot()
{
    return false;
}

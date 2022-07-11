#include "handlestuffnwaccess.h"

HandleStuffNWAccess::HandleStuffNWAccess() : HandleStuff ()
{
    load = false;
    connect(emuclient, &EmuNWAccessClient::readyRead, this, &HandleStuffNWAccess::onReplyRead);
}

void HandleStuffNWAccess::setNWAClient(EmuNWAccessClient *client)
{
    emuclient = client;
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
    emuclient->cmdSaveState(path);
    emuclient->waitForReadyRead();
    auto reply = emuclient->readReply();
    return !reply.isError;
}

bool HandleStuffNWAccess::loadState(QString path)
{
    load = true;
    emuclient->cmdLoadState(path);
    return true;
}

bool HandleStuffNWAccess::needByteData()
{
    return false;
}

void HandleStuffNWAccess::onReplyRead()
{
    if (load)
        emit loadStateFinished(!emuclient->readReply().isError);
    else
        emit saveStateFinished(!emuclient->readReply().isError);
}

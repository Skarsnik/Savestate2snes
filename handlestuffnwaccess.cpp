#include "handlestuffnwaccess.h"

HandleStuffNWAccess::HandleStuffNWAccess() : HandleStuff ()
{
    load = false;
}

void HandleStuffNWAccess::setNWAClient(EmuNWAccessClient *client)
{
    emuclient = client;
    connect(emuclient, &EmuNWAccessClient::readyRead, this, &HandleStuffNWAccess::onReplyRead);
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
    return true;
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


bool HandleStuffNWAccess::hasPostSaveScreenshot()
{
    return false;
}

bool HandleStuffNWAccess::doScreenshot()
{
    return false;
}

#include "handlestuffusb2snes.h"

Q_LOGGING_CATEGORY(log_handleUSB2SNES, "HSUsb2snes")

#define sDebug() qCDebug(log_handleUSB2SNES)

HandleStuffUsb2snes::HandleStuffUsb2snes() : HandleStuff()
{

}


void HandleStuffUsb2snes::setUsb2snes(USB2snes *usbsnes)
{
    usb2snes = usbsnes;
}

//$FC2000 db saveState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2001 db loadState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2002 dw saveButton (Set to $FFFF first and then set to correct value)
//$FC2004 dw loadButton (Set to $FFFF first and then set to correct value)

bool HandleStuffUsb2snes::saveState(bool trigger)
{
    QByteArray data;
    if (trigger)
    {
        checkForSafeState();
        data.resize(2);
        data[0] = 0;
        /*data[1] = 0;
        usb2snes->setAddress(0xFC2000, data);*/
        data[0] = 1;
        usb2snes->setAddress(0xFC2000, data);
    }
    checkForSafeState();
    saveStateData = usb2snes->getAddress(0xF00000, 320 * 1024);
    QTimer::singleShot(10, this, SIGNAL(saveStateFinished(true)));
    return true;
}

void HandleStuffUsb2snes::loadState(QByteArray data)
{
    checkForSafeState();
    usb2snes->setAddress(0xF00000, data);
    data.resize(2);
    data[0] = 0;
    /*data[1] = 0;
    usb2snes->setAddress(0xFC2000, data);*/
    data[1] = 1;
    usb2snes->setAddress(0xFC2000, data);
    QTimer::singleShot(10, this, SIGNAL(loadStateFinished(true)));
}

bool HandleStuffUsb2snes::needByteData()
{
    return true;
}

void    HandleStuffUsb2snes::checkForSafeState()
{
    QByteArray data = usb2snes->getAddress(0xFC2000, 2);
    while (!(data.at(0) == 0 && data.at(1) == 0))
    {
        QThread::usleep(100);
        data = usb2snes->getAddress(0xFC2000, 2);
    }
}

quint16 HandleStuffUsb2snes::shortcutSave()
{
    QByteArray saveButton = usb2snes->getAddress(0xFC2002, 2);
    sDebug() << "Shortcut save from usb2snes" << saveButton;
    quint16 toret = (static_cast<uchar>(saveButton.at(1)) << 8) + static_cast<uchar>(saveButton.at(0));
    return toret;
}

quint16 HandleStuffUsb2snes::shortcutLoad()
{
    QByteArray loadButton = usb2snes->getAddress(0xFC2004, 2);
    sDebug() << "Shortcut load from usb2snes" << loadButton;
    quint16 toret = (static_cast<uchar>(loadButton.at(1)) << 8) + static_cast<uchar>(loadButton.at(0));
    return toret;
}

QByteArray HandleStuffUsb2snes::getScreenshotData()
{
    return QByteArray();
}

void HandleStuffUsb2snes::setShortcutLoad(quint16 shortcut)
{
    QByteArray data;
    data.resize(2);
    data[0] = static_cast<char>(shortcut & 0x00FF);
    data[1] = static_cast<char>(shortcut >> 8);
    usb2snes->setAddress(0xFC2004, data);
}

void HandleStuffUsb2snes::setShortcutSave(quint16 shortcut)
{
    QByteArray data;
    data.resize(2);
    data[0] = static_cast<char>(shortcut & 0x00FF);
    data[1] = static_cast<char>(shortcut >> 8);
    usb2snes->setAddress(0xFC2002, data);
}

bool HandleStuffUsb2snes::hasScreenshots()
{
    return false;
}

bool HandleStuffUsb2snes::hasShortcutsEdit()
{
    return true;
}



bool HandleStuffUsb2snes::saveState(QString path)
{
    Q_UNUSED(path)
    return false;
}

bool HandleStuffUsb2snes::loadState(QString path)
{
    Q_UNUSED(path)
    return false;
}

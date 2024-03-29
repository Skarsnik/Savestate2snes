#include "handlestuffusb2snes.h"

Q_LOGGING_CATEGORY(log_handleUSB2SNES, "HSUsb2snes")

#define sDebug() qCDebug(log_handleUSB2SNES)

HandleStuffUsb2snes::HandleStuffUsb2snes() : HandleStuff()
{
    doingLoadState = false;
    doingSaveState = false;
    doingMemoryWatchCheck = false;
    memoryWatchTimer.setInterval(20);
    safeStateTimer.setInterval(30);
    connect(&memoryWatchTimer, &QTimer::timeout, this, [=] {
        doingMemoryWatchCheck = true;
        usb2snes->getAsyncAddress(GameInfos().memoryPreset.address + 0xF50000, GameInfos().memoryPreset.size);
    });
    connect(&safeStateTimer, &QTimer::timeout, this, [=] {
        usb2snes->getAsyncAddress(0xFC2000, 2);
    });
}


void HandleStuffUsb2snes::setUsb2snes(USB2snes *usbsnes)
{
    usb2snes = usbsnes;
    connect(usb2snes, &USB2snes::getAddressDataReceived, this, &HandleStuffUsb2snes::onGetAddressDataReceived);
}

//$FC2000 db saveState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2001 db loadState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2002 dw saveButton (Set to $FFFF first and then set to correct value)
//$FC2004 dw loadButton (Set to $FFFF first and then set to correct value)

bool HandleStuffUsb2snes::saveState(bool trigger)
{
    QByteArray data;
    doingSaveState = true;
    saveStateTrigger = trigger;
    checkForSafeState();
    return true;
}

void HandleStuffUsb2snes::loadState(QByteArray data)
{
    sDebug() << "Loading State";
    doingLoadState = true;
    loadStateData = data;
    checkForSafeState();
}

bool HandleStuffUsb2snes::needByteData()
{
    return true;
}

void    HandleStuffUsb2snes::checkForSafeState()
{
    safeStateTimer.start();
}

void HandleStuffUsb2snes::onGetAddressDataReceived()
{
    const QByteArray& data = usb2snes->getAsyncAdressData();
    sDebug() << "Received" << data.toHex('-') << safeStateTimer.isActive();
    if (doingMemoryWatchCheck)
    {
        quint64 value = 0;
        const QByteArray& data = usb2snes->getAsyncAdressData();
        sDebug() << data;
        for (quint8 i = 0; i < data.size(); i++)
        {
            value += data.at(i) << (i * 8);
        }
        emit gotMemoryValue(value);
        return ;
    }
    if (safeStateTimer.isActive())
    {
        if (data.at(0) == 0 && data.at(1) == 0)
        {
            sDebug() << "Ready to load/save state";
            safeStateTimer.stop();
            if (doingLoadState)
            {
                usb2snes->setAddress(0xF00000, loadStateData);
                loadStateData.resize(2);
                loadStateData[0] = 0;
                loadStateData[1] = 1;
                usb2snes->setAddress(0xFC2000, loadStateData);
                doingLoadState = false;
                emit loadStateFinished(true);
                return ;
            }
            if (doingSaveState)
            {
                sDebug() << "Doing savestate";
                if (saveStateTrigger)
                {
                    loadStateData.resize(1);
                    loadStateData[0] = 1;
                    usb2snes->setAddress(0xFC2000, loadStateData);
                    safeStateTimer.start();
                    saveStateTrigger = false;
                } else {
                    usb2snes->getAsyncAddress(0xF00000, 320 * 1024);
                }
                return;
            }
        }
    }
    if (doingSaveState)
    {
        sDebug() << "Receiving savestate data";
        saveStateData = data;
        doingSaveState = false;
        emit saveStateFinished(true);
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

bool HandleStuffUsb2snes::hasMemoryWatch()
{
    return false;
}

void HandleStuffUsb2snes::startMemoryWatch()
{
    memoryWatchTimer.start();
    return ;
}

void HandleStuffUsb2snes::stopMemoryWatch()
{
    memoryWatchTimer.stop();
    return ;
}

void HandleStuffUsb2snes::controllerSaveState()
{

}

void HandleStuffUsb2snes::controllerLoadState()
{

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


bool HandleStuffUsb2snes::hasPostSaveScreenshot()
{
    return false;
}

bool HandleStuffUsb2snes::doScreenshot()
{
    return false;
}

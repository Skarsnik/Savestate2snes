#include "handlestuffusb2snes.h"

Q_LOGGING_CATEGORY(log_handleUSB2SNES, "HSUsb2snes")

extern bool    dontLogNext;
#define sDebug() qCDebug(log_handleUSB2SNES)

HandleStuffUsb2snes::HandleStuffUsb2snes() : HandleStuff()
{
    detectedSaveState = false;
    detectedLoadState = false;
    doingMemoryWatchCheck = false;
    savestateInterfaceAddress = 0xFC2000;
    savestateDataAddress = 0xF00000;
    m_loadShortcut = 0;
    m_saveShortcut = 0;
    //safeStateTimer.setInterval(30);
    /*connect(&safeStateTimer, &QTimer::timeout, this, [=] {
        usb2snes->getAsyncAddress(savestateInterfaceAddress, 2);
    });*/
}


void HandleStuffUsb2snes::setUsb2snes(USB2snes *usbsnes)
{
    usb2snes = usbsnes;
    connect(usb2snes, &USB2snes::getAddressDataReceived, this, &HandleStuffUsb2snes::onGetAddressDataReceived);
    connect(usb2snes, &USB2snes::stateChanged, this, [=] {
        if (usb2snes->state() == USB2snes::Ready)
        {
            sDebug() << "Setting offset address";
            if (usb2snes->firmwareVersion() >= QVersionNumber(11))
            {
                savestateInterfaceAddress = 0xFE1000;
                savestateDataAddress = 0xF00000;
            } else {
                savestateInterfaceAddress = 0xFC2000;
                savestateDataAddress = 0xF00000;
            }
        }
    });
}

void HandleStuffUsb2snes::savestateReady()
{
    sDebug() << "Ready, starting interface check";
    memoryToCheck << QPair<quint32, quint8>(savestateInterfaceAddress, 0x16);
    usb2snes->getAsyncAddress(memoryToCheck);
}

void HandleStuffUsb2snes::savestateUnready()
{
    sDebug() << "Unready, stoping interface check";
    memoryToCheck.clear();
}

//$FC2000 db saveState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2001 db loadState  (Make sure both load and save are zero before setting this to nonzero)
//$FC2002 dw saveButton (Set to $FFFF first and then set to correct value)
//$FC2004 dw loadButton (Set to $FFFF first and then set to correct value)

bool HandleStuffUsb2snes::saveState(bool trigger)
{
    QByteArray data;
    triggeredState = WAITING_FOR_SAFE_SAVE;
    saveStateTrigger = trigger;
    return true;
}

void HandleStuffUsb2snes::loadState(QByteArray data)
{
    sDebug() << "Loading State";
    triggeredState = WAITING_FOR_SAFE_LOAD;
    loadStateData = data;
}

bool HandleStuffUsb2snes::needByteData()
{
    return true;
}


void HandleStuffUsb2snes::onGetAddressDataReceived()
{
    const QByteArray& data = usb2snes->getAsyncAdressData();
    //sDebug() << "Received" << data.toHex('-') << checkingSafeState;

    if (triggeredState == WAITING_FOR_SAFE_LOAD)
    {
        if (data.at(0) == 0 && data.at(1) == 0)
        {
            usb2snes->setAddress(savestateDataAddress, loadStateData);
            loadStateData.resize(2);
            loadStateData[0] = 0;
            loadStateData[1] = 1;
            usb2snes->setAddress(savestateInterfaceAddress, loadStateData);
            triggeredState = NONE;
            emit loadStateFinished(true);
        }
    }
    if (triggeredState == TRIGGERED_SAVE)
    {
        if (data.at(0) == 0 && data.at(1) == 0)
        {
            usb2snes->getAsyncAddress(savestateDataAddress, 320 * 1024);
            triggeredState = WAITING_FOR_DATA;
            return ;
        }
    }
    if (triggeredState == WAITING_FOR_SAFE_SAVE)
    {
        if (data.at(0) == 0 && data.at(1) == 0)
        {
            if (saveStateTrigger)
            {
                loadStateData[0] = 1;
                usb2snes->setAddress(savestateInterfaceAddress, loadStateData);
                triggeredState = TRIGGERED_SAVE;
            } else {
                usb2snes->getAsyncAddress(savestateDataAddress, 320 * 1024);
                triggeredState = WAITING_FOR_DATA;
                return ;
            }
        }
    }
    if (triggeredState == WAITING_FOR_DATA)
    {
        sDebug() << "Receiving savestate data";
        saveStateData = data;
        triggeredState = NONE;
        emit saveStateFinished(true);
        usb2snes->getAsyncAddress(memoryToCheck);
        return ;
    }
    if (doingMemoryWatchCheck)
    {
        quint64 value = 0;
        QByteArray memData = data.mid(0x16);
        //sDebug() << "Memory watch" << memData;
        for (quint8 i = 0; i < memData.size(); i++)
        {
            value += memData.at(i) << (i * 8);
        }
        emit gotMemoryValue(value);
    }
    /* This is actually not really accurate, that detect a savestate action is
     * performed, only way to detect wich one would be to check the value
     * of button, but it's stay for one frame, another way should be to check
     * for the current cmd in the CMD space, but it's a separate read
    */
    //sDebug() << data.size() << data.at(0xC);
    if (data.at(0xC) == 0 && detectedLoadState)
    {
        detectedLoadState = false;
        emit controllerLoadStateFinished(true);
    }

    if (data.at(0xC) != 0 && detectedLoadState == false)
    {
         sDebug() << "Load state triggered";
        detectedLoadState = true;
    }
    if (memoryToCheck.isEmpty() == false)
    {
        m_loadShortcut = (static_cast<uchar>(data.at(5)) << 8) + static_cast<uchar>(data.at(4));
        m_saveShortcut = (static_cast<uchar>(data.at(3)) << 8) + static_cast<uchar>(data.at(2));
        usb2snes->getAsyncAddress(memoryToCheck);
    }
}

quint16 HandleStuffUsb2snes::shortcutSave()
{
    return m_saveShortcut;
}

quint16 HandleStuffUsb2snes::shortcutLoad()
{
    return m_loadShortcut;
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
    usb2snes->setAddress(savestateInterfaceAddress + 4, data);
}

void HandleStuffUsb2snes::setShortcutSave(quint16 shortcut)
{
    QByteArray data;
    data.resize(2);
    data[0] = static_cast<char>(shortcut & 0x00FF);
    data[1] = static_cast<char>(shortcut >> 8);
    usb2snes->setAddress(savestateInterfaceAddress + 2, data);
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
    return true;
}

void HandleStuffUsb2snes::startMemoryWatch()
{
    sDebug() << "Starting memory watch";
    memoryToCheck << QPair<quint32, quint8>(gameInfos().memoryPreset.address + 0xF50000, gameInfos().memoryPreset.size);
    doingMemoryWatchCheck = true;
    return ;
}

void HandleStuffUsb2snes::stopMemoryWatch()
{
    sDebug() << "Stoping memory watch";
    memoryToCheck.removeLast();
    doingMemoryWatchCheck = false;
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

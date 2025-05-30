#ifndef HANDLESTUFFUSB2SNES_H
#define HANDLESTUFFUSB2SNES_H

#include "handlestuff.h"
#include "usb2snes.h"

class HandleStuffUsb2snes : public HandleStuff
{
public:
    HandleStuffUsb2snes();
    void        setUsb2snes(USB2snes* usb);
    void        savestateReady();
    void        savestateUnready();
    quint16     shortcutSave();
    quint16     shortcutLoad();
    QByteArray  getScreenshotData();
    void        setShortcutLoad(quint16 shortcut);
    void        setShortcutSave(quint16 shortcut);
    bool        hasScreenshots();
    bool        hasShortcutsEdit();
    bool        hasMemoryWatch();
    void        startMemoryWatch();
    void        stopMemoryWatch();
    void        controllerSaveState();
    void        controllerLoadState();

protected:
    bool        saveState(bool trigger);
    void        loadState(QByteArray data);
    bool        needByteData();
    bool        saveState(QString path);
    bool        loadState(QString path);
    bool        hasPostSaveScreenshot();
    bool        doScreenshot();


private:
    USB2snes*   usb2snes;
    bool        doingMemoryWatchCheck;
    bool        saveStateTrigger;
    bool        detectedLoadState;
    bool        detectedSaveState;
    bool        checkingSafeState;
    QList<QPair<quint32, quint8> > memoryToCheck;
    //QTimer      safeStateTimer;
    QByteArray  loadStateData;
    quint32     savestateInterfaceAddress;
    quint32     savestateDataAddress;
    quint16     m_loadShortcut;
    quint16     m_saveShortcut;

    enum TriggeredSaveState {
        NONE,
        WAITING_FOR_SAFE_LOAD,
        WAITING_FOR_SAFE_SAVE,
        TRIGGERED_SAVE,
        TRIGGERED_LOAD,
        WAITING_FOR_DATA
    } triggeredState = NONE;

    void        onGetAddressDataReceived();

};

#endif // HANDLESTUFFUSB2SNES_H

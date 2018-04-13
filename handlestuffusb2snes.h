#ifndef HANDLESTUFFUSB2SNES_H
#define HANDLESTUFFUSB2SNES_H

#include "handlestuff.h"
#include "usb2snes.h"

class HandleStuffUsb2snes : public HandleStuff
{
public:
    HandleStuffUsb2snes();
    void        setUsb2snes(USB2snes* usb);
    quint16     shortcutSave();
    quint16     shortcutLoad();
    QByteArray  getScreenshotData();
    void        setShortcutLoad(quint16 shortcut);
    void        setShortcutSave(quint16 shortcut);
    bool        hasScreenshots();
    bool        hasShortcutsEdit();

protected:
    QByteArray  saveState(bool trigger);
    void        loadState(QByteArray data);

private:
    USB2snes*   usb2snes;
    void checkForSafeState();
};

#endif // HANDLESTUFFUSB2SNES_H

#ifndef HANDLENWACCESS_H
#define HANDLENWACCESS_H

#include "handlestuff.h"
#include "emunwaccessclient.h"

#include <QTimer>

class HandleStuffNWAccess : public HandleStuff
{
public:
    HandleStuffNWAccess();

    // HandleStuff interface
public:
    void    setNWAClient(EmuNWAccessClient* client);
    QByteArray getScreenshotData();
    bool    hasShortcutsEdit();
    bool    hasScreenshots();
    void    setShortcutSave(quint16 shortcut);
    void    setShortcutLoad(quint16 shortcut);
    quint16 shortcutSave();
    quint16 shortcutLoad();
    bool    hasMemoryWatch();
    void    startMemoryWatch();
    void    stopMemoryWatch();
    void        controllerLoadState();
    void        controllerSaveState();

protected:
    bool saveState(bool trigger);
    void loadState(QByteArray data);
    bool saveState(QString path);
    bool loadState(QString path);
    bool needByteData();
    bool hasPostSaveScreenshot();
    bool doScreenshot();

private:
    EmuNWAccessClient*  emuclient;
    bool                load;
    bool                doingState;
    bool                memoryAccess;
    bool                controllerStateRequest;
    QString             tempFilePath;
    QTimer              memoryTimer;
    QString             lastLoadedSave;
    quint16                 saveShortcut;
    quint16                 loadShortcut;

    void    onReplyRead();

};

#endif // HANDLENWACCESS_H

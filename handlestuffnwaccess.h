#ifndef HANDLENWACCESS_H
#define HANDLENWACCESS_H

#include "handlestuff.h"
#include "emunwaccessclient.h"

class HandleStuffNWAccess : public HandleStuff
{
public:
    HandleStuffNWAccess();

    // HandleStuff interface
public:
    void    setNWAClient(EmuNWAccessClient* client);
    QByteArray getScreenshotData();
    bool hasShortcutsEdit();
    bool hasScreenshots();
    void setShortcutSave(quint16 shortcut);
    void setShortcutLoad(quint16 shortcut);
    quint16 shortcutSave();
    quint16 shortcutLoad();

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

    void    onReplyRead();

};

#endif // HANDLENWACCESS_H

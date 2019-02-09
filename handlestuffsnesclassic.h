#ifndef HANDLESTUFFSNESCLASSIC_H
#define HANDLESTUFFSNESCLASSIC_H

#include "handlestuff.h"
#include "stuffclient.h"

class HandleStuffSnesClassic : public HandleStuff
{
public:
    HandleStuffSnesClassic();
    void            setControlCo(StuffClient* client);
    void            setShortcutLoad(quint16 shortcut);
    void            setShortcutSave(quint16 shortcut);
    QByteArray      getScreenshotData();
    quint16         shortcutLoad();
    quint16         shortcutSave();
    bool            hasScreenshots();
    bool            hasShortcutsEdit();
    void            controllerSaveState();
    void            controllerLoadState();

protected:
    QByteArray      saveState(bool trigger);
    void            loadState(QByteArray data);


private:
    StuffClient*            controlCo;
    QByteArray              lastLoadMD5;
    quint16                 saveShortcut;
    quint16                 loadShortcut;

    void        runCanoe(QStringList canoeArgs);
    QStringList getCanoeExecution();
    void        killCanoe(int signal);
    void        removeCanoeUnnecessaryArg(QStringList &canoeRun);
    QByteArray  mySaveState(bool trigger, bool noGet);
    void        myLoadState(QByteArray data, bool noPut);
};

#endif // HANDLESTUFFSNESCLASSIC_H
